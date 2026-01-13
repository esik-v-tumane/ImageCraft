#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "convolution.h"
#include "defines.h"

Kernel* kernel_create(uint8_t size, float** matrix) {
    Kernel* w = (Kernel*)malloc(sizeof(Kernel));
    if (!w) {
        fprintf(stderr, "Ошибка инициализации ядра\n");
        return NULL;
    }

    w->matrix = (float**)malloc(size * sizeof(float*));
    if (!w->matrix) {
        fprintf(stderr, "Ошибка инициализации матрицы ядра\n");
        free(w);
        return NULL;
    }

    w->size = size;
    w->normalizer = 0;

    // Инициализация матрицы
    for (uint8_t i = 0; i < size; i++) {
        w->matrix[i] = (float*)malloc(size * sizeof(float));
        if (!(w->matrix[i])) {
            fprintf(
                stderr,
                "Ошибка инициализации вектора %u\n",
                i
            );
            // Освобождаем уже выделенную память
            for (uint8_t j = 0; j < i; j++) {
                free(w->matrix[j]);
            }
            free(w->matrix);
            free(w);
            return NULL;
        }
    }

    // Копирование значений матрицы
    for (uint8_t i = 0; i < size; i++) {
        for (uint8_t j = 0; j < size; j++) {
            w->matrix[i][j] =
                matrix[i][j]; // TODO: ДОбавить обработку
                              // некорректных размеров
            w->normalizer += matrix[i][j];
        }
    }

    return w;
}

void kernel_free(Kernel* w) {
    if (!w)
        return;

    for (uint8_t i = 0; i < w->size; i++)
        free(w->matrix[i]);

    free(w);
}

int kernel_fill(Kernel* w, float** matrix, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        for (uint8_t j = 0; j < size; j++) {
            w->matrix[i][j] = matrix[i][j];
        }
    }
    return 0;
}

void kernel_print(Kernel* w) {
    uint8_t n = w->size;
    for (uint8_t i = 0; i < n; i++) {
        printf("| ");
        for (uint8_t j = 0; j < n; j++) {
            printf("%f ", w->matrix[i][j]);
        }
        printf("|\n");
    }
}

void convolute_pixel(
    BMPImage* iimage,
    BMPImage* oimage,
    Kernel* w,
    int32_t i,
    int32_t j
) {
    // Проверка аргументов
    if (!w || !iimage || !iimage->pixels || !oimage ||
        !oimage->pixels) {
        printf("Ошибка: некорректные аргументы\n");
        return;
    }

    int32_t width = iimage->info_header.width;
    int32_t height = iimage->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    // Проверка границ
    if (i < 0 || i >= abs_height || j < 0 || j >= width) {
        printf(
            "Ошибка: координаты (%" PRId32 ", %" PRId32
            ") вне границ изображения %dx%d\n",
            i,
            j,
            width,
            abs_height
        );
        return;
    }

    if (w->size % 2 != 1) {
        printf(
            "Ошибка: порядок ядра должен быть нечётным, "
            "получено %u\n",
            w->size
        );
        return;
    }

    // Вычисляем смещение для окрестности 3x3
    int32_t Z = (w->size - 1) / 2; // = 1 для 3x3

    // Массивы для хранения значений каналов
    float red_result = 0, green_result = 0, blue_result = 0;

    // Собираем значения из окрестности
    for (int32_t p = -Z; p <= Z; p++) {
        for (int32_t q = -Z; q <= Z; q++) {
            // Вычисляем координаты с учётом границ
            int32_t row = i + p;
            int32_t col = j + q;

            // Применяем граничные условия "зеркало"
            if (row < 0)
                row = 0; // Верхняя граница
            else if (row >= abs_height)
                row = abs_height - 1; // Нижняя граница

            if (col < 0)
                col = 0; // Левая граница
            else if (col >= width)
                col = width - 1; // Правая граница

            int32_t access_row =
                (height > 0) ? (abs_height - 1 - row) : row;
            RGBPixel pixel = iimage->pixels[access_row][col];

            // Вычисляем свёртку
            red_result += pixel.red * w->matrix[p + Z][q + Z];
            green_result +=
                pixel.green * w->matrix[p + Z][q + Z];
            blue_result += pixel.blue * w->matrix[p + Z][q + Z];
        }
    }

    // Нормализация (если нормализатор не ноль)
    if (w->normalizer != 0) {
        red_result /= w->normalizer;
        green_result /= w->normalizer;
        blue_result /= w->normalizer;
    }

    // ограничение диапазона до 0..255
    red_result = (red_result < 0) ? 0
        : (red_result > 255)      ? 255
                                  : red_result;
    green_result = (green_result < 0) ? 0
        : (green_result > 255)        ? 255
                                      : green_result;
    blue_result = (blue_result < 0) ? 0
        : (blue_result > 255)       ? 255
                                    : blue_result;

    uint8_t final_red = (uint8_t)red_result;
    uint8_t final_green = (uint8_t)green_result;
    uint8_t final_blue = (uint8_t)blue_result;

    // Окраска пикселя
    bmp_set_pixel(
        oimage,
        j,
        i,
        final_red,
        final_green,
        final_blue
    );
}

BMPImage* convolute(BMPImage* iimage, Kernel* w) {
    // Проверка аргументов
    if (!w || !iimage || !iimage->pixels) {
        fprintf(stderr, "[Ошибка] Некорректные аргументы\n");
        return NULL;
    }

    // Копирование изображения
    BMPImage* oimage = bmp_copy(iimage);
    if (!oimage) {
        fprintf(stderr, "Ошибка копирования изображения\n");
        return NULL;
    }

    int32_t width = iimage->info_header.width;
    int32_t height = iimage->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    for (int32_t i = 0; i < abs_height; i++) {
        for (int32_t j = 0; j < width; j++)
            convolute_pixel(iimage, oimage, w, i, j);
    }

    return oimage;
}
