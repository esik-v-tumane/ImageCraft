#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "bmp.h"
#include <stdint.h>

typedef struct {
    uint8_t size;     // порядок матрицы
    float** matrix;   // матрица ядра
    float normalizer; // контроль яркости
} Kernel;

Kernel* kernel_create(uint8_t size, float** matrix);

void kernel_free(Kernel* w);

void kernel_print(Kernel* w);

void convolute_pixel(
    BMPImage* iimage,
    BMPImage* oimage,
    Kernel* w,
    int32_t i,
    int32_t j
);

BMPImage* convolute(BMPImage* iimage, Kernel* w);

#endif // !CONVOLUTION_H
