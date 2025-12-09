#include <stdint.h>
#include <stdlib.h>

#include "args_parser.h"
#include "bmp.h"
#include "convolution.h"
#include "core.h"
#include "defines.h"
#include "help.h"

void test_convolute_simple() {
    // Создаем простое тестовое изображение 10x10
    BMPImage* test_img = bmp_create(10, 10);

    // Заполняем градиентом
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            uint8_t r = x * 25;
            uint8_t g = y * 25;
            uint8_t b = 128;
            bmp_set_pixel(test_img, x, y, r, g, b);
        }
    }

    // Простое ядро
    float matrix[3][3] = { { 0, -1, 0 },
                           { -1, 5, -1 },
                           { 0, -1, 0 } };
    float* rows[3];
    for (int i = 0; i < 3; i++)
        rows[i] = matrix[i];

    Kernel* k = kernel_create(3, rows);
    printf("Kernel sum: %f\n", k->normalizer);

    // Проверяем несколько пикселей вручную
    printf("\nManual check of original pixels:\n");
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int32_t row = (test_img->info_header.height > 0)
                ? (10 - 1 - y)
                : y;
            RGBPixel p = test_img->pixels[row][x];
            printf(
                "(%d,%d): R=%d,G=%d,B=%d [row=%d]\n",
                x,
                y,
                p.red,
                p.green,
                p.blue,
                row
            );
        }
    }

    // Применяем свёртку
    BMPImage* result = convolute(test_img, k);
    bmp_save(result, "test_output.bmp");

    bmp_free(test_img);
    bmp_free(result);
    kernel_free(k);
}

void create_test_image() {
    BMPImage* test = bmp_create(100, 100);

    // Левая половина - белая, правая - черная
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            if (x < 50) {
                bmp_set_pixel(
                    test,
                    x,
                    y,
                    255,
                    255,
                    255
                ); // Белый
            } else {
                bmp_set_pixel(test, x, y, 0, 0, 0); // Черный
            }
        }
    }

    bmp_save(test, "test_edge.bmp");

    // Примените ядро краевого детектора
    float edge_matrix[3][3] = { { -1, -1, -1 },
                                { -1, 8, -1 },
                                { -1, -1, -1 } };
    float* rows[3];
    for (int i = 0; i < 3; i++)
        rows[i] = edge_matrix[i];

    Kernel* edge_kernel = kernel_create(3, rows);
    // BMPImage* edges = convolute(test, edge_kernel);
    bmp_save(test, "edges_detected.bmp");

    bmp_free(test);
    // bmp_free(edges);
    kernel_free(edge_kernel);
}

// TODO: Удалить^

int imagecraft(int argc, char** argv) {
    char *ifile = NULL, *ofile = NULL;
    parse_args(argc, argv, &ifile, &ofile);

    if (argc == 1) {
        printhelp();
        return 0;
    }

    int error;
    if (((error = bmp_is_valid_24bit(ifile)) & 1) == 1) {
        if (error == IC_ERROR_OPENING_FILE)
            fprintf(
                stderr,
                "[Error] Не получилось прочитать файл '%s'\n",
                ifile
            );
        else // иначе проблема в структуре BMP
        {
            char* error_code;
            fprintf(
                stderr,
                "[Error] Некорректный файл BMP (код ошибки "
            );

            // Определение кода ошибки
            if (error == IC_BMP_ERROR_NULL_FILENAME)
                error_code = IC_MESSAGE_BMP_ERROR_NULL_FILENAME;
            else if (error == IC_BMP_ERROR_INVALID_SIGNATURE)
                error_code =
                    IC_MESSAGE_BMP_ERROR_INVALID_SIGNATURE;
            else if (error == IC_BMP_ERROR_INVALID_BPP)
                error_code = IC_MESSAGE_BMP_ERROR_INVALID_BPP;
            else if (error == IC_BMP_ERROR_INVALID_DIB)
                error_code = IC_MESSAGE_BMP_ERROR_INVALID_DIB;

            fprintf(stderr, "%s)\n", error_code);
        }
        return error;
    }

    // Загрузка изображения
    BMPImage* image = bmp_load(ifile);
    if (!image) {
        fprintf(
            stderr,
            "[Error] Не удалось загрузить изображение '%s'\n",
            ifile
        );
        return 1;
    }

    bmp_print_info(image);

    // 2. ПРОВЕРКА НЕСКОЛЬКИХ ПИКСЕЛЕЙ (для отладки)
    printf("\n=== Проверка загруженного изображения ===\n");
    int32_t width = image->info_header.width;
    int32_t height = image->info_header.height;
    int32_t abs_height = height < 0 ? -height : height;

    // Покажем угловые пиксели
    printf("Левый верхний угол (0,0): ");
    RGBPixel p = bmp_get_pixel(image, 0, 0);
    printf("R=%d, G=%d, B=%d\n", p.red, p.green, p.blue);

    printf("Правый верхний угол (%d,0): ", width - 1);
    p = bmp_get_pixel(image, width - 1, 0);
    printf("R=%d, G=%d, B=%d\n", p.red, p.green, p.blue);

    printf(
        "Центр изображения (%d,%d): ",
        width / 2,
        abs_height / 2
    );
    p = bmp_get_pixel(image, width / 2, abs_height / 2);
    printf("R=%d, G=%d, B=%d\n", p.red, p.green, p.blue);

    // 3. ВЫБОР ЯДРА (предложите пользователю выбрать или
    // используйте аргументы)
    Kernel* w = NULL;

    // Пример: ядро размытия 5x5
    float matrix[3][3] = { { -1, -1, -1 },
                           { -1, 8, -1 },
                           { -1, -1, -1 } };
    float* rows[3];
    for (int i = 0; i < 3; i++)
        rows[i] = matrix[i];

    w = kernel_create(3, rows);

    if (!w) {
        fprintf(stderr, "[Error] Не удалось создать ядро\n");
        bmp_free(image);
        return 1;
    }

    printf("\n=== Применение ядра ===\n");
    printf("Создано ядро:\n");
    kernel_print(w);
    printf("Сумма ядра: %f\n", w->normalizer);

    // Применение свёртки
    printf("\nПрименяю свёртку...\n");
    BMPImage* result = convolute(image, w);

    if (!result) {
        fprintf(stderr, "[Error] Свёртка не удалась\n");
        kernel_free(w);
        bmp_free(image);
        return 1;
    }

    // СОХРАНЕНИЕ РЕЗУЛЬТАТА
    // Если выходной файл не указан, используем "output.bmp"
    if (!ofile) {
        ofile = "output.bmp";
        printf(
            "\nВыходной файл не указан, сохраняю в '%s'\n",
            ofile
        );
    }

    int save_result = bmp_save(result, ofile);
    if (save_result == ALL_OK) {
        printf("Изображение успешно сохранено в '%s'!\n", ofile);
    } else {
        fprintf(
            stderr,
            "Ошибка сохранения изображения (код: %d)\n",
            save_result
        );
    }

    // 7. ОЧИСТКА ПАМЯТИ
    bmp_free(image);
    bmp_free(result);
    kernel_free(w);

    return ALL_OK;
}
