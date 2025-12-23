#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "args_parser.h"
#include "bmp.h"
#include "convolution.h"
#include "core.h"
#include "defines.h"
#include "filters.h"
#include "help.h"

static int create_output_directory(const char* path) {
    struct stat st = { 0 };
    if (stat(path, &st) == -1) {
// Директория не существует, создаём её
#ifdef _WIN32
        return mkdir(path);
#else
        return mkdir(path, 0755); // права на чтение/запись для
                                  // владельца
#endif
    }
    return 0; // Директория уже существует
}

int imagecraft(int argc, char** argv) {
    char *ifile = NULL, *ofile = NULL;
    Filter* filter = (Filter*)malloc(sizeof(Filter));
    int parse_result =
        parse_args(argc, argv, &ifile, &ofile, filter);

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
                "[Error] Файл %s не соответствует формату BMP "
                "(код ошибки ",
                ifile
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

    Kernel* w = NULL;

    float matrix[3][3] = { { -255, -255, -255 },
                           { -255, 2040, -255 },
                           { -255, -255, -255 } };
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

    // Сохранение результата
    if (!ofile) {
        // Если выходной файл не указан, используем OUTFILE
        ofile = SLASH SAVE_DIR SLASH OUTFILE;
        printf(
            "\nВыходной файл не указан, сохраняю в "
            "'%s'\n",
            ofile
        );
    }

    // Создаём директорию SAVE_DIR, если её нет
    const char* dir_path = SLASH SAVE_DIR;
    if (create_output_directory(dir_path) != 0) {
        fprintf(
            stderr,
            "[Error] Не удалось создать директорию '%s'\n",
            dir_path
        );
        bmp_free(result);
        kernel_free(w);
        bmp_free(image);
        return 1;
    }

    int save_result = bmp_save(result, ofile);
    if (save_result == ALL_OK) {
        printf("Изображение успешно сохранено в '%s'!\n", ofile);
    } else {
        fprintf(
            stderr,
            "[Error] Ошибка сохранения изображения (код: %d)\n",
            save_result
        );
    }

    // Очистка памяти
    bmp_free(image);
    bmp_free(result);
    kernel_free(w);

    return ALL_OK;
}
