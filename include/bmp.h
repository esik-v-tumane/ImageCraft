#ifndef BMP_READER_H
#define BMP_READER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#pragma pack(push, 1) // Выключаем выравнивание структур для
                      // правильного чтения BMP

// Структура BMP заголовка файла
typedef struct {
    uint16_t signature; // Сигнатура 'BM'
    uint32_t file_size; // Размер файла в байтах
    uint16_t reserved1; // Зарезервировано (0)
    uint16_t reserved2; // Зарезервировано (0)
    uint32_t
        data_offset; // Смещение до начала данных изображения
} BMPFileHeader;

// Структура DIB заголовка (BITMAPINFOHEADER)
typedef struct {
    uint32_t header_size;    // Размер этой структуры (40 байт)
    int32_t width;           // Ширина изображения в пикселях
    int32_t height;          // Высота изображения в пикселях
                             // (положительное - снизу вверх)
    uint16_t planes;         // Число плоскостей (1)
    uint16_t bits_per_pixel; // Бит на пиксель (24)
    uint32_t compression; // Тип сжатия (0 - BI_RGB, без сжатия)
    uint32_t image_size;  // Размер данных изображения в байтах
                          // (0 или реальный размер)
    int32_t x_pixels_per_meter; // Горизонтальное разрешение
                                // (пикселей на метр)
    int32_t y_pixels_per_meter; // Вертикальное разрешение
                                // (пикселей на метр)
    uint32_t colors_used; // Число используемых цветов в палитре
                          // (0 - нет палитры)
    uint32_t
        colors_important; // Число важных цветов (0 - все важны)
} BMPInfoHeader;

// Структура для хранения пикселя RGB (24 бита)
typedef struct {
    uint8_t blue;  // Синий канал
    uint8_t green; // Зеленый канал
    uint8_t red;   // Красный канал
} RGBPixel;

// Структура для представления BMP изображения
typedef struct {
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    RGBPixel**
        pixels; // Двумерный массив пикселей [height][width]
} BMPImage;

#pragma pack(pop) // Восстанавливаем выравнивание

// Функции для работы с BMP

// Создание нового BMP изображения
BMPImage* bmp_create(int32_t width, int32_t height);

// Загрузка BMP изображения из файла
BMPImage* bmp_load(const char* filename);

// Сохранение BMP изображения в файл
int bmp_save(BMPImage* image, const char* filename);

// Очистка памяти, занятой изображением
void bmp_free(BMPImage* image);

// Установка цвета пикселя
void bmp_set_pixel(
    BMPImage* image,
    int32_t x,
    int32_t y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
);

// Получение цвета пикселя
RGBPixel
bmp_get_pixel(const BMPImage* image, int32_t x, int32_t y);

// Создание копии изображения
BMPImage* bmp_copy(const BMPImage* src);

// Проверка, является ли файл валидным 24-битным BMP
int bmp_is_valid_24bit(const char* filename);

// Вывод информации о BMP изображении
void bmp_print_info(const BMPImage* image);

void printinfo(const char* filename);

#endif // BMP_READER_H
