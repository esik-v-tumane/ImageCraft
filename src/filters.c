#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convolution.h"
#include "defines.h"
#include "filters.h"

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ====================

float rgb_to_grayscale(uint8_t r, uint8_t g, uint8_t b) {
    return 0.299f * r + 0.587f * g + 0.114f * b;
}

uint8_t clamp_float_to_uint8(float value) {
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return (uint8_t)value;
}

uint8_t clamp_int_to_uint8(int value) {
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return (uint8_t)value;
}

// ==================== РЕАЛИЗАЦИИ ФИЛЬТРОВ ====================

// Фильтр обрезки (crop)
BMPImage* filter_crop(BMPImage* image, int width, int height) {
    if (!image || width <= 0 || height <= 0) {
        return NULL;
    }

    int32_t img_width = image->info_header.width;
    int32_t img_height = image->info_header.height;
    int32_t abs_img_height =
        img_height < 0 ? -img_height : img_height;

    // Если запрошенные размеры больше исходных, используем
    // доступную часть
    int crop_width = (width > img_width) ? img_width : width;
    int crop_height =
        (height > abs_img_height) ? abs_img_height : height;

    // Создаем новое изображение нужного размера
    BMPImage* cropped = bmp_create(crop_width, crop_height);
    if (!cropped) {
        return NULL;
    }

    // Копируем пиксели из верхнего левого угла
    for (int y = 0; y < crop_height; y++) {
        for (int x = 0; x < crop_width; x++) {
            RGBPixel pixel = bmp_get_pixel(image, x, y);
            bmp_set_pixel(
                cropped,
                x,
                y,
                pixel.red,
                pixel.green,
                pixel.blue
            );
        }
    }

    return cropped;
}

// Фильтр оттенков серого (grayscale)
int filter_grayscale(BMPImage* image) {
    if (!image) {
        return 1;
    }

    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    for (int y = 0; y < abs_height; y++) {
        for (int x = 0; x < width; x++) {
            RGBPixel pixel = bmp_get_pixel(image, x, y);
            uint8_t gray = (uint8_t)rgb_to_grayscale(
                pixel.red,
                pixel.green,
                pixel.blue
            );
            bmp_set_pixel(image, x, y, gray, gray, gray);
        }
    }

    return 0;
}

// Фильтр негатива (negative)
int filter_negative(BMPImage* image) {
    if (!image) {
        return 1;
    }

    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    for (int y = 0; y < abs_height; y++) {
        for (int x = 0; x < width; x++) {
            RGBPixel pixel = bmp_get_pixel(image, x, y);
            bmp_set_pixel(
                image,
                x,
                y,
                255 - pixel.red,
                255 - pixel.green,
                255 - pixel.blue
            );
        }
    }

    return 0;
}

// Фильтр повышения резкости (sharpening)
int filter_sharpening(BMPImage* image) {
    if (!image) {
        return 1;
    }

    // Матрица для повышения резкости
    float sharp_matrix[3][3] = { { 0, -1, 0 },
                                 { -1, 5, -1 },
                                 { 0, -1, 0 } };

    float* rows[3];
    for (int i = 0; i < 3; i++) {
        rows[i] = sharp_matrix[i];
    }

    Kernel* kernel = kernel_create(3, rows);
    if (!kernel) {
        return 1;
    }

    BMPImage* result = convolute(image, kernel);
    if (!result) {
        kernel_free(kernel);
        return 1;
    }

    // Копируем результат обратно в исходное изображение
    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    for (int y = 0; y < abs_height; y++) {
        for (int x = 0; x < width; x++) {
            RGBPixel pixel = bmp_get_pixel(result, x, y);
            bmp_set_pixel(
                image,
                x,
                y,
                pixel.red,
                pixel.green,
                pixel.blue
            );
        }
    }

    bmp_free(result);
    kernel_free(kernel);
    return 0;
}

// Фильтр выделения границ (edge detection)
BMPImage*
filter_edge_detection(BMPImage* image, float threshold) {
    if (!image) {
        return NULL;
    }

    // Сначала преобразуем в оттенки серого
    BMPImage* gray_image = bmp_copy(image);
    if (!gray_image) {
        return NULL;
    }

    filter_grayscale(gray_image);

    // Матрица для выделения границ
    float edge_matrix[3][3] = { { 0, -1, 0 },
                                { -1, 4, -1 },
                                { 0, -1, 0 } };

    float* rows[3];
    for (int i = 0; i < 3; i++) {
        rows[i] = edge_matrix[i];
    }

    Kernel* kernel = kernel_create(3, rows);
    if (!kernel) {
        bmp_free(gray_image);
        return NULL;
    }

    BMPImage* edges = convolute(gray_image, kernel);
    bmp_free(gray_image);
    kernel_free(kernel);

    if (!edges) {
        return NULL;
    }

    // Применяем порог
    int32_t width = edges->info_header.width;
    int32_t height = edges->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    float threshold_value = threshold * 255.0f;

    for (int y = 0; y < abs_height; y++) {
        for (int x = 0; x < width; x++) {
            RGBPixel pixel = bmp_get_pixel(edges, x, y);
            float gray = rgb_to_grayscale(
                pixel.red,
                pixel.green,
                pixel.blue
            );

            if (gray > threshold_value) {
                bmp_set_pixel(
                    edges,
                    x,
                    y,
                    255,
                    255,
                    255
                ); // \u0411\u0435\u043b\u044b\u0439
            } else {
                bmp_set_pixel(
                    edges,
                    x,
                    y,
                    0,
                    0,
                    0
                ); // \u0427\u0435\u0440\u043d\u044b\u0439
            }
        }
    }

    return edges;
}

// Вспомогательная функция для создания ядра Гаусса
static Kernel* create_gaussian_kernel(int size, float sigma) {
    if (size % 2 == 0) {
        return NULL;
    }

    float** matrix = (float**)malloc(size * sizeof(float*));
    if (!matrix) {
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        matrix[i] = (float*)malloc(size * sizeof(float));
        if (!matrix[i]) {
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }

    float sum = 0.0f;
    int half = size / 2;

    for (int i = -half; i <= half; i++) {
        for (int j = -half; j <= half; j++) {
            float value =
                expf(-(i * i + j * j) / (2 * sigma * sigma));
            matrix[i + half][j + half] = value;
            sum += value;
        }
    }

    // Нормализация
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] /= sum;
        }
    }

    Kernel* kernel = kernel_create(size, matrix);

    // Освобождаем временную матрицу
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return kernel;
}

// Фильтр Гауссова размытия (gaussian blur)
BMPImage* filter_gaussian_blur(BMPImage* image, float sigma) {
    if (!image || sigma <= 0) {
        return NULL;
    }

    // Определяем размер ядра (минимум 3x3, максимум 11x11)
    int kernel_size = (int)(sigma * 6) | 1; // Нечетное число
    if (kernel_size < 3)
        kernel_size = 3;
    if (kernel_size > 11)
        kernel_size = 11;

    Kernel* kernel = create_gaussian_kernel(kernel_size, sigma);
    if (!kernel) {
        return NULL;
    }

    BMPImage* blurred = convolute(image, kernel);
    kernel_free(kernel);

    return blurred;
}

// Вспомогательная функция для сравнения значений (для медианного
// фильтра)
static int compare_uint8(const void* a, const void* b) {
    return (*(uint8_t*)a - *(uint8_t*)b);
}

// Фильтр медианной фильтрации (median filter)
BMPImage* filter_median(BMPImage* image, int window) {
    if (!image || window % 2 == 0 || window < 3) {
        return NULL;
    }

    BMPImage* result = bmp_copy(image);
    if (!result) {
        return NULL;
    }

    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    int half = window / 2;
    int total_pixels = window * window;

    // Выделяем память для массивов значений
    uint8_t* reds =
        (uint8_t*)malloc(total_pixels * sizeof(uint8_t));
    uint8_t* greens =
        (uint8_t*)malloc(total_pixels * sizeof(uint8_t));
    uint8_t* blues =
        (uint8_t*)malloc(total_pixels * sizeof(uint8_t));

    if (!reds || !greens || !blues) {
        free(reds);
        free(greens);
        free(blues);
        bmp_free(result);
        return NULL;
    }

    for (int y = 0; y < abs_height; y++) {
        for (int x = 0; x < width; x++) {
            int count = 0;

            // Собираем значения из окна
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    // Применяем граничные условия
                    if (nx < 0)
                        nx = 0;
                    if (nx >= width)
                        nx = width - 1;
                    if (ny < 0)
                        ny = 0;
                    if (ny >= abs_height)
                        ny = abs_height - 1;

                    RGBPixel pixel =
                        bmp_get_pixel(image, nx, ny);
                    reds[count] = pixel.red;
                    greens[count] = pixel.green;
                    blues[count] = pixel.blue;
                    count++;
                }
            }

            // Находим медианные значения
            qsort(reds, count, sizeof(uint8_t), compare_uint8);
            qsort(greens, count, sizeof(uint8_t), compare_uint8);
            qsort(blues, count, sizeof(uint8_t), compare_uint8);

            uint8_t median_red = reds[count / 2];
            uint8_t median_green = greens[count / 2];
            uint8_t median_blue = blues[count / 2];

            bmp_set_pixel(
                result,
                x,
                y,
                median_red,
                median_green,
                median_blue
            );
        }
    }

    free(reds);
    free(greens);
    free(blues);

    return result;
}

// ==================== ОСНОВНАЯ ФУНКЦИЯ ПРИМЕНЕНИЯ ФИЛЬТРОВ
// ====================

int apply_filters(BMPImage** image, Filter* filter_list) {
    if (!image || !*image) {
        return 0;
    }

    if (!filter_list) {
        return 0; // Список фильтров пустой
    }

    Filter* current = filter_list;
    int count = 0;

    while (current) {
        count++;

        switch (current->type) {
            case ARGV_TYPE_FILTER_CROP: {
                int width = current->params[0];
                int height = current->params[1];
                BMPImage* cropped =
                    filter_crop(*image, width, height);

                if (!cropped) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_CROP "\n"
                    );
                    return -count;
                }

                bmp_free(*image);
                *image = cropped;
                break;
            }

            case ARGV_TYPE_FILTER_GS: {
                if (filter_grayscale(*image) != 0) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_GS " \n"
                    );
                    return -count;
                }
                break;
            }

            case ARGV_TYPE_FILTER_NEG: {
                if (filter_negative(*image) != 0) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_NEG "\n"
                    );
                    return -count;
                }
                break;
            }

            case ARGV_TYPE_FILTER_SHARP: {
                if (filter_sharpening(*image) != 0) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_SHARP "\n"
                    );
                    return -count;
                }
                break;
            }

            case ARGV_TYPE_FILTER_EDGE: {
                float threshold = current->params[0] / 1000.0f;
                BMPImage* edges =
                    filter_edge_detection(*image, threshold);

                if (!edges) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_EDGE "\n"
                    );
                    return -count;
                }

                bmp_free(*image);
                *image = edges;
                break;
            }

            case ARGV_TYPE_FILTER_BLUR: {
                float sigma = current->params[0] / 1000.0f;
                BMPImage* blurred =
                    filter_gaussian_blur(*image, sigma);

                if (!blurred) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_BLUR "-blur\n"
                    );
                    return -count;
                }

                bmp_free(*image);
                *image = blurred;
                break;
            }

            case ARGV_TYPE_FILTER_MED: {
                int window = current->params[0];
                BMPImage* median = filter_median(*image, window);

                if (!median) {
                    fprintf(
                        stderr,
                        "[Error] Не удалось применить "
                        "фильтр " IC_ARGV_FILTER_MED "\n"
                    );
                    return -count;
                }

                bmp_free(*image);
                *image = median;
                break;
            }

            default:
                fprintf(
                    stderr,
                    "[Error] Неизвестный тип фильтра: %d\n",
                    current->type
                );
                return -count;
        }

        printf("[Info] Применен фильтр #%d\n", count);
        current = current->next;
    }

    return count;
}
