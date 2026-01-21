#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "args_assistant.h"
#include "bmp.h"
#include "core.h"
#include "defines.h"
#include "filters.h"
#include "paths.h"

int imagecraft(int argc, char** argv) {
    char *ifile = NULL, *ofile = NULL;
    Filter* filter_list = NULL;
    int parse_result =
        parse_args(argc, argv, &ifile, &ofile, &filter_list);

    switch (parse_result) {
        case IC_ARGS_ASSISTANT_ERROR: {
            fprintf(
                stderr,
                "[Error] Ошибка обработка аргументов. Adiós!\n"
            );
            exit(1);
        }
        case IC_ARGS_ASSISTANT_HELP: {
            printhelp();
            exit(0);
        }
        case IC_ARGS_ASSISTANT_VERSION: {
            printversion();
            exit(0);
        }
        case IC_ARGS_ASSISTANT_INFO: {
            printinfo(ifile);
            exit(0);
        }
    }

    if (argc < 3) {
        printhelp();
        exit(0);
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
            else // if (error == IC_BMP_ERROR_INVALID_DIB)
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

    // bmp_print_info(image);

    int filters_applied = apply_filters(&image, filter_list);

    // Результат применения фильтров
    if (filters_applied < 0) {
        // Ошибка применения фильтров
        fprintf(
            stderr,
            "[Error] Ошибка применения фильтров на шаге %d\n",
            -filters_applied
        );

        // Очистка памяти
        bmp_free(image);
        free_filter_list(filter_list);

        return 1;
    } else if (filters_applied > 0) {
        printf(
            "[Info] Успешно применено фильтров: %d\n",
            filters_applied
        );
    } else {
        printf("[Info] Фильтры не применялись\n");
    }

    // Сохранение результата
    if (!ofile) {
        // Если выходной файл не указан, используем OUTFILE
        char default_path[1024];
        snprintf(
            default_path,
            sizeof(default_path),
            "%s%s%s",
            SAVE_DIR,
            SLASH,
            OUTFILE
        );
        ofile = ic_strdup(default_path); // Делаем копию строки
        printf(
            "\n[Success] Выходной файл не указан, сохраняю в "
            "'%s'\n",
            ofile
        );
    } else {
        // Пользователь указал путь, делаем копию для
        // безопасности
        ofile = ic_strdup(ofile);
    }

    // Извлекаем директорию из пути и создаем её
    char* output_dir = get_directory_from_path(ofile);

    // Если в пути есть директория — создаём её рекурсивно
    if (output_dir != NULL && strcmp(output_dir, ofile) != 0) {
        if (create_output_directory_recursive(output_dir) != 0) {
            fprintf(
                stderr,
                "[Error] Не удалось создать директорию '%s'\n",
                output_dir
            );

            // Очистка памяти
            bmp_free(image);
            free_filter_list(filter_list);
            free(ofile);

            return 1;
        }
    }
    /*
    // Проверяем, не является ли путь просто именем файла (без
    // директории)
    if (strcmp(output_dir, ofile) != 0) {
        // Если есть поддиректории, создаем их
        if (create_output_directory_recursive(output_dir) != 0) {
            fprintf(
                stderr,
                "[Error] Не удалось создать директорию '%s'\n",
                output_dir
            );

            // Очистка памяти
            bmp_free(image);
            free_filter_list(filter_list);
            free(ofile);

            return 1;
        }
    }
    */

    int save_result = bmp_save(image, ofile);

    // Освобождаем память для ofile
    free(ofile);

    if (save_result == ALL_OK) {
        printf("[Success] Изображение успешно сохранено!\n");
    } else {
        fprintf(
            stderr,
            "[Error] Ошибка сохранения изображения (код: %d)\n",
            save_result
        );

        // Очистка памяти
        bmp_free(image);
        free_filter_list(filter_list);

        return 1;
    }

    // Очистка памяти
    bmp_free(image);
    free_filter_list(filter_list);

    return ALL_OK;
}
