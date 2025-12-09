#include <stdio.h>

#include "bmp.h"
#include "defines.h"

#include <stdlib.h>
#include <string.h>

// Вспомогательная функция для вычисления размера строки с
// выравниванием
static uint32_t calculate_row_size(int32_t width) {
    // Размер строки в байтах должен быть кратен 4
    uint32_t row_size = width * 3; // 3 байта на пиксель (RGB)
    uint32_t padding = (4 - (row_size % 4)) % 4;
    return row_size + padding;
}

// Создание нового BMP изображения
BMPImage* bmp_create(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return NULL;
    }

    // Выделяем память для структуры изображения
    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    if (!image) {
        return NULL;
    }

    // Заполняем файловый заголовок
    image->file_header.signature = 0x4D42; // 'BM'
    image->file_header.reserved1 = 0;
    image->file_header.reserved2 = 0;
    image->file_header.data_offset =
        sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    // Заполняем информационный заголовок
    image->info_header.header_size = sizeof(BMPInfoHeader);
    image->info_header.width = width;
    image->info_header.height = height;
    image->info_header.planes = 1;
    image->info_header.bits_per_pixel = 24;
    image->info_header.compression = 0;           // BI_RGB
    image->info_header.x_pixels_per_meter = 2835; // ~72 DPI
    image->info_header.y_pixels_per_meter = 2835; // ~72 DPI
    image->info_header.colors_used = 0;
    image->info_header.colors_important = 0;

    // Вычисляем размер строки с учетом выравнивания
    uint32_t row_size = calculate_row_size(width);
    image->info_header.image_size =
        row_size * (height < 0 ? -height : height);
    image->file_header.file_size =
        image->file_header.data_offset +
        image->info_header.image_size;

    // Выделяем память для массива указателей на строки
    int32_t abs_height = height < 0 ? -height : height;
    image->pixels =
        (RGBPixel**)malloc(abs_height * sizeof(RGBPixel*));
    if (!image->pixels) {
        free(image);
        return NULL;
    }

    // Выделяем память для всех пикселей
    image->pixels[0] =
        (RGBPixel*)malloc(abs_height * width * sizeof(RGBPixel));
    if (!image->pixels[0]) {
        free(image->pixels);
        free(image);
        return NULL;
    }

    // Инициализируем указатели на строки
    for (int32_t i = 1; i < abs_height; i++) {
        image->pixels[i] = image->pixels[i - 1] + width;
    }

    // Зануляем все пиксели (черный цвет)
    memset(
        image->pixels[0],
        0,
        abs_height * width * sizeof(RGBPixel)
    );

    return image;
}

// Загрузка BMP изображения из файла
BMPImage* bmp_load(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    // Читаем файловый заголовок
    BMPFileHeader file_header;
    if (fread(&file_header, sizeof(BMPFileHeader), 1, file) !=
        1) {
        fclose(file);
        return NULL;
    }

    // Проверяем сигнатуру
    if (file_header.signature != 0x4D42) { // 'BM'
        fclose(file);
        return NULL;
    }

    // Читаем DIB заголовок
    BMPInfoHeader info_header;
    if (fread(&info_header, sizeof(BMPInfoHeader), 1, file) !=
        1) {
        fclose(file);
        return NULL;
    }

    // Проверяем, что это BITMAPINFOHEADER (размер 40)
    if (info_header.header_size != 40) {
        fclose(file);
        return NULL;
    }

    // Проверяем, что это 24-битное изображение без сжатия
    if (info_header.bits_per_pixel != 24 ||
        info_header.compression != 0) {
        fclose(file);
        return NULL;
    }

    // Проверяем ширину и высоту
    if (info_header.width <= 0 || info_header.height == 0) {
        fclose(file);
        return NULL;
    }

    // Создаем структуру изображения
    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    if (!image) {
        fclose(file);
        return NULL;
    }

    // Копируем заголовки
    image->file_header = file_header;
    image->info_header = info_header;

    int32_t width = info_header.width;
    int32_t height = info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    // Выделяем память для массива указателей на строки
    image->pixels =
        (RGBPixel**)malloc(abs_height * sizeof(RGBPixel*));
    if (!image->pixels) {
        free(image);
        fclose(file);
        return NULL;
    }

    // Выделяем память для всех пикселей
    image->pixels[0] =
        (RGBPixel*)malloc(abs_height * width * sizeof(RGBPixel));
    if (!image->pixels[0]) {
        free(image->pixels);
        free(image);
        fclose(file);
        return NULL;
    }

    // Инициализируем указатели на строки
    for (int32_t i = 1; i < abs_height; i++) {
        image->pixels[i] = image->pixels[i - 1] + width;
    }

    // Переходим к данным изображения
    if (fseek(file, file_header.data_offset, SEEK_SET) != 0) {
        bmp_free(image);
        fclose(file);
        return NULL;
    }

    // Вычисляем размер строки с учетом выравнивания
    uint32_t row_size = calculate_row_size(width);

    // Читаем данные изображения
    uint8_t* row_buffer = (uint8_t*)malloc(row_size);
    if (!row_buffer) {
        bmp_free(image);
        fclose(file);
        return NULL;
    }

    // Определяем порядок строк (в BMP снизу вверх, если высота
    // положительная)
    int start_row, end_row, step;
    if (height > 0) {
        // Изображение снизу вверх
        start_row = abs_height - 1;
        end_row = -1;
        step = -1;
    } else {
        // Изображение сверху вниз (отрицательная высота)
        start_row = 0;
        end_row = abs_height;
        step = 1;
        height = -height; // Делаем высоту положительной для
                          // упрощения
    }

    // Читаем строки
    for (int32_t row = start_row; row != end_row; row += step) {
        if (fread(row_buffer, 1, row_size, file) != row_size) {
            free(row_buffer);
            bmp_free(image);
            fclose(file);
            return NULL;
        }

        // Копируем пиксели из буфера строки
        for (int32_t col = 0; col < width; col++) {
            image->pixels[row][col].blue = row_buffer[col * 3];
            image->pixels[row][col].green =
                row_buffer[col * 3 + 1];
            image->pixels[row][col].red =
                row_buffer[col * 3 + 2];
        }
    }

    free(row_buffer);
    fclose(file);
    return image;
}

// Сохранение BMP изображения в файл
int bmp_save(BMPImage* image, const char* filename) {
    if (!image || !filename) {
        return IC_BMP_ERROR_SAVING_FILE;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        return IC_ERROR_OPENING_FILE;
    }

    // Записываем заголовки
    if (fwrite(
            &image->file_header,
            sizeof(BMPFileHeader),
            1,
            file
        ) != 1 ||
        fwrite(
            &image->info_header,
            sizeof(BMPInfoHeader),
            1,
            file
        ) != 1) {
        fclose(file);
        return IC_BMP_ERROR_WRITING_HEADER;
    }

    // Вычисляем размер строки с учетом выравнивания
    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;
    uint32_t row_size = calculate_row_size(width);

    // Подготавливаем буфер для строки с выравниванием
    uint8_t* row_buffer = (uint8_t*)calloc(row_size, 1);
    if (!row_buffer) {
        fclose(file);
        return IC_BMP_ERROR_ALLOCATING_BUFFER;
    }

    // Определяем порядок записи строк
    int start_row, end_row, step;
    if (height > 0) {
        // Записываем снизу вверх
        start_row = abs_height - 1;
        end_row = -1;
        step = -1;
    } else {
        // Записываем сверху вниз
        start_row = 0;
        end_row = abs_height;
        step = 1;
        height = -height;
    }

    // Записываем данные изображения
    for (int32_t row = start_row; row != end_row; row += step) {
        // Копируем пиксели в буфер строки
        for (int32_t col = 0; col < width; col++) {
            row_buffer[col * 3] = image->pixels[row][col].blue;
            row_buffer[col * 3 + 1] =
                image->pixels[row][col].green;
            row_buffer[col * 3 + 2] =
                image->pixels[row][col].red;
        }

        // Записываем строку с выравниванием
        if (fwrite(row_buffer, 1, row_size, file) != row_size) {
            free(row_buffer);
            fclose(file);
            return IC_BMP_ERROR_WRITING_ROW;
        }
    }

    free(row_buffer);
    fclose(file);
    return ALL_OK;
}

// Очистка памяти, занятой изображением
void bmp_free(BMPImage* image) {
    if (image) {
        if (image->pixels) {
            if (image->pixels[0]) {
                free(image->pixels[0]);
            }
            free(image->pixels);
        }
        free(image);
    }
}

// Установка цвета пикселя
void bmp_set_pixel(
    BMPImage* image,
    int32_t x,
    int32_t y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
) {
    if (!image || !image->pixels) {
        return;
    }

    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    if (x < 0 || x >= width || y < 0 || y >= abs_height) {
        return;
    }

    // Если высота положительная, изображение хранится снизу
    // вверх
    int32_t row = (height > 0) ? (abs_height - 1 - y) : y;

    image->pixels[row][x].red = red;
    image->pixels[row][x].green = green;
    image->pixels[row][x].blue = blue;
}

// Получение цвета пикселя
RGBPixel
bmp_get_pixel(const BMPImage* image, int32_t x, int32_t y) {
    RGBPixel pixel = { 0, 0, 0 };

    if (!image || !image->pixels) {
        return pixel;
    }

    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    if (x < 0 || x >= width || y < 0 || y >= abs_height) {
        return pixel;
    }

    // Если высота положительная, изображение хранится снизу
    // вверх
    int32_t row = (height > 0) ? (abs_height - 1 - y) : y;

    return image->pixels[row][x];
}

// Создание копии изображения
BMPImage* bmp_copy(const BMPImage* src) {
    if (!src) {
        return NULL;
    }

    // Создаем новое изображение такого же размера
    BMPImage* dst = bmp_create(
        src->info_header.width,
        src->info_header.height
    );
    if (!dst) {
        return NULL;
    }

    // Копируем заголовки
    dst->file_header = src->file_header;
    dst->info_header = src->info_header;

    // Копируем пиксели
    int32_t width = src->info_header.width;
    int32_t height = src->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    for (int32_t y = 0; y < abs_height; y++) {
        for (int32_t x = 0; x < width; x++) {
            dst->pixels[y][x] = src->pixels[y][x];
        }
    }

    return dst;
}

// Проверка, является ли файл валидным 24-битным BMP
int bmp_is_valid_24bit(const char* filename) {
    if (filename == NULL)
        return IC_BMP_ERROR_NULL_FILENAME;

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return IC_ERROR_OPENING_FILE;
    }

    // Читаем сигнатуру
    uint16_t signature;
    if (fread(&signature, sizeof(uint16_t), 1, file) != 1) {
        fclose(file);
        return IC_BMP_ERROR_INVALID_SIGNATURE;
    }

    if (signature != 0x4D42) { // 'BM'
        fclose(file);
        return IC_BMP_ERROR_INVALID_SIGNATURE;
    }

    // Переходим к DIB заголовку
    if (fseek(file, 14, SEEK_SET) != 0) {
        fclose(file);
        return IC_BMP_ERROR_INVALID_DIB;
    }

    // Читаем размер DIB заголовка
    uint32_t header_size;
    if (fread(&header_size, sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return IC_BMP_ERROR_INVALID_DIB;
    }

    if (header_size != 40) { // BITMAPINFOHEADER
        fclose(file);
        return IC_BMP_ERROR_INVALID_DIB;
    }

    // Читаем bits_per_pixel
    if (fseek(file, 28, SEEK_SET) !=
        0) { // смещение до bits_per_pixel
        fclose(file);
        return IC_BMP_ERROR_INVALID_BPP;
    }

    uint16_t bits_per_pixel;
    if (fread(&bits_per_pixel, sizeof(uint16_t), 1, file) != 1) {
        fclose(file);
        return IC_BMP_ERROR_INVALID_BPP;
    }

    fclose(file);
    return !(bits_per_pixel == 24);
}

// Вывод информации о BMP изображении
void bmp_print_info(const BMPImage* image) {
    if (!image) {
        printf("Image is NULL\n");
        return;
    }

    printf("=== BMP Image Info ===\n");
    printf(
        "Signature: 0x%X ('%c%c')\n",
        image->file_header.signature,
        image->file_header.signature & 0xFF,
        image->file_header.signature >> 8
    );
    printf(
        "File size: %u bytes\n",
        image->file_header.file_size
    );
    printf(
        "Data offset: %u bytes\n",
        image->file_header.data_offset
    );
    printf(
        "Header size: %u bytes\n",
        image->info_header.header_size
    );
    printf(
        "Image dimensions: %d x %d pixels\n",
        image->info_header.width,
        image->info_header.height
    );
    printf(
        "Bits per pixel: %d\n",
        image->info_header.bits_per_pixel
    );
    printf(
        "Compression: %u (0 = BI_RGB, no compression)\n",
        image->info_header.compression
    );
    printf(
        "Image data size: %u bytes\n",
        image->info_header.image_size
    );
    printf("Colors used: %u\n", image->info_header.colors_used);
    printf(
        "Colors important: %u\n",
        image->info_header.colors_important
    );

    int32_t abs_height = image->info_header.height < 0
        ? -image->info_header.height
        : image->info_header.height;

    printf(
        "Orientation: %s\n",
        image->info_header.height > 0 ? "bottom-to-top"
                                      : "top-to-bottom"
    );
    printf(
        "Memory used: %lu bytes\n",
        abs_height * image->info_header.width * sizeof(RGBPixel)
    );
    printf("========================\n");
}
