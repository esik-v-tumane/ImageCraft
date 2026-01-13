#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_assistant.h"
#include "defines.h"
#include "filters.h"

// Вспомогательная функция для проверки, является ли строка целым
// числом
static int is_integer(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0; // Пустая строка
    }

    // Пропускаем знак, если есть
    const char* p = str;
    if (*p == '-' || *p == '+') {
        p++;
    }

    // Проверяем, что все символы - цифры
    while (*p != '\0') {
        if (!isdigit(*p)) {
            return 0; // Не цифра
        }
        p++;
    }

    return 1; // Все символы - цифры
}

// Вспомогательная функция для проверки, является ли строка
// числом с плавающей точкой
static int is_float(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }

    const char* p = str;
    int has_digit = 0;
    int has_dot = 0;

    // Пропускаем знак
    if (*p == '-' || *p == '+') {
        p++;
    }

    // Проверяем цифры и точку
    while (*p != '\0') {
        if (isdigit(*p)) {
            has_digit = 1;
        } else if (*p == '.') {
            if (has_dot) {
                return 0; // Больше одной точки
            }
            has_dot = 1;
        } else {
            return 0; // Не цифра и не точка
        }
        p++;
    }

    return has_digit; // Должна быть хотя бы одна цифра
}

// Вспомогательная функция для создания нового фильтра
static Filter*
create_filter(int type, int param_count, int params[]) {
    Filter* new_filter = (Filter*)malloc(sizeof(Filter));
    if (!new_filter) {
        return NULL;
    }

    new_filter->type = type;
    new_filter->param_count = param_count;

    // Копируем параметры
    for (int i = 0; i < param_count && i < MAX_FILTER_PARAMS;
         i++) {
        new_filter->params[i] = params[i];
    }

    new_filter->next = NULL;
    return new_filter;
}

int parse_args(
    int argc,
    char** argv,
    char** ifile,
    char** ofile,
    Filter** head // Изменилось на двойной указатель!
) {
    // Инициализируем список как пустой
    *head = NULL;

    if (argc < 2) {
        return 0; // Нет аргументов, только вызов программы
    }

    // Если первый аргумент -help
    if (strcmp(argv[1], IC_ARGV_HELP) == 0) {
        return IC_ARGS_ASSISTANT_HELP; // печатай подсказку
    } else if (strcmp(argv[1], IC_ARGV_VERSION) == 0) {
        // или если -version
        return IC_ARGS_ASSISTANT_VERSION; // печатай версию
    }

    // Если аргумент -info
    if (strcmp(argv[1], IC_ARGV_INFO) == 0) {
        if (argc > 2) {
            *ifile = argv[2];
            return IC_ARGS_ASSISTANT_INFO;
        } else {
            fprintf(
                stderr,
                "[Error] Не указан файл для чтения!\n"
            );
            return IC_ARGS_ASSISTANT_ERROR;
        }
    }

    // Иначе первый аргумент - входной файл
    *ifile = argv[1];

    // TODO: Добавить обработку -help, -version, -info в середине
    // списка аргументов

    // Второй аргумент (если есть) - выходной файл
    if (argc >= 3 && argv[2][0] != '-') {
        *ofile = argv[2];
    } else {
        // Если второй аргумент начинается с '-', значит это
        // фильтр
        *ofile = NULL;
    }

    // Обрабатываем фильтры, начиная с нужного индекса
    int start_index = (*ofile) ? 3 : 2;

    for (int i = start_index; i < argc; i++) {
        if (strcmp(argv[i], IC_ARGV_FILTER_CROP) == 0) {
            // Проверяем, что есть достаточно аргументов
            if (i + 2 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_CROP
                    " ожидает 2 аргумента: width, "
                    "height.\n"
                );
                // Освобождаем уже созданные фильтры при ошибке
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Проверяем, что аргументы - целые числа
            if (!is_integer(argv[i + 1]) ||
                !is_integer(argv[i + 2])) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_CROP
                    " ожидает целые числа для "
                    "width и height\n"
                );
                fprintf(
                    stderr,
                    "        Получено: width='%s', "
                    "height='%s'\n",
                    argv[i + 1],
                    argv[i + 2]
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Парсим ширину и высоту
            int width = atoi(argv[i + 1]);
            int height = atoi(argv[i + 2]);

            // Проверка корректности размеров
            if (width <= 0 || height <= 0) {
                fprintf(
                    stderr,
                    "[Error] Размеры для обрезки должны быть "
                    "положительными\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Создаем фильтр обрезки
            int params[2] = { width, height };
            Filter* crop_filter =
                create_filter(ARGV_TYPE_FILTER_CROP, 2, params);

            if (!crop_filter) {
                fprintf(
                    stderr,
                    "[Error] Не удалось выделить память для "
                    "фильтра\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Добавляем в список
            if (*head == NULL) {
                *head = crop_filter;
            } else {
                // Находим последний элемент
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = crop_filter;
            }

            // Переходим к следующему фильтру
            i += 2; // Пропускаем обработанные аргументы

        } else if (strcmp(argv[i], IC_ARGV_FILTER_GS) == 0) {
            // Фильтр grayscale без параметров
            Filter* gs_filter =
                create_filter(ARGV_TYPE_FILTER_GS, 0, NULL);

            // Добавляем в список
            if (*head == NULL) {
                *head = gs_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = gs_filter;
            }

        } else if (strcmp(argv[i], IC_ARGV_FILTER_NEG) == 0) {
            // Фильтр negative без параметров
            Filter* neg_filter =
                create_filter(ARGV_TYPE_FILTER_NEG, 0, NULL);

            // Добавляем в список
            if (*head == NULL) {
                *head = neg_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = neg_filter;
            }

        } else if (strcmp(argv[i], IC_ARGV_FILTER_SHARP) == 0) {
            // Фильтр sharpening без параметров
            Filter* sharp_filter =
                create_filter(ARGV_TYPE_FILTER_SHARP, 0, NULL);

            // Добавляем в список
            if (*head == NULL) {
                *head = sharp_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = sharp_filter;
            }

        } else if (strcmp(argv[i], IC_ARGV_FILTER_EDGE) == 0) {
            // Фильтр edge detection с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_EDGE
                    " ожидает 1 аргумент: "
                    "threshold\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            float threshold = atof(argv[i + 1]);
            // Сохраняем threshold как целое (умножаем на 1000
            // для точности)
            int threshold_int = (int)(threshold * 1000);
            int params[1] = { threshold_int };
            Filter* edge_filter =
                create_filter(ARGV_TYPE_FILTER_EDGE, 1, params);

            // Добавляем в список
            if (*head == NULL) {
                *head = edge_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = edge_filter;
            }

            i += 1; // Пропускаем параметр

        } else if (strcmp(argv[i], IC_ARGV_FILTER_BLUR) == 0) {
            // Фильтр blur с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_BLUR
                    " ожидает 1 аргумент: sigma\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            float sigma = atof(argv[i + 1]);
            // Сохраняем sigma как целое (умножаем на 1000 для
            // точности)
            int sigma_int = (int)(sigma * 1000);
            int params[1] = { sigma_int };
            Filter* blur_filter =
                create_filter(ARGV_TYPE_FILTER_BLUR, 1, params);

            // Добавляем в список
            if (*head == NULL) {
                *head = blur_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = blur_filter;
            }

            i += 1; // Пропускаем параметр

        } else if (strcmp(argv[i], IC_ARGV_FILTER_MED) == 0) {
            // Фильтр median с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_MED
                    " ожидает 1 аргумент: window\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            int window = atoi(argv[i + 1]);
            if (window <= 0 || window % 2 == 0) {
                fprintf(
                    stderr,
                    "[Error] window должен быть положительным "
                    "нечетным числом\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            int params[1] = { window };
            Filter* med_filter =
                create_filter(ARGV_TYPE_FILTER_MED, 1, params);

            // Добавляем в список
            if (*head == NULL) {
                *head = med_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = med_filter;
            }

            i += 1; // Пропускаем параметр

        } else if (strcmp(argv[i], IC_ARGV_FILTER_VIGNETTE) ==
                   0) {
            // Фильтр vignette с двумя параметрами
            if (i + 2 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_VIGNETTE
                    " ожидает 2 аргумента: intensity (0.0-1.0), "
                    "radius (0.0-1.0)\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Проверяем, что аргументы - числа с плавающей
            // точкой
            if (!is_float(argv[i + 1]) ||
                !is_float(argv[i + 2])) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_VIGNETTE
                    " ожидает числа с плавающей точкой\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            float intensity = atof(argv[i + 1]);
            float radius = atof(argv[i + 2]);

            // Проверка диапазонов
            if (intensity < 0.0f || intensity > 1.0f ||
                radius < 0.0f || radius > 1.0f) {
                fprintf(
                    stderr,
                    "[Error] Параметры vignette должны быть в "
                    "диапазоне [0.0, 1.0]\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Сохраняем параметры (умножаем на 1000 для
            // точности)
            int params[2] = { (int)(intensity * 1000),
                              (int)(radius * 1000) };

            Filter* vignette_filter = create_filter(
                ARGV_TYPE_FILTER_VIGNETTE,
                2,
                params
            );

            // Добавляем в список
            if (*head == NULL) {
                *head = vignette_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = vignette_filter;
            }

            i += 2; // Пропускаем параметры

        } else if (strcmp(argv[i], IC_ARGV_FILTER_ZOOM) == 0) {
            // Фильтр zoom blur с тремя параметрами
            if (i + 3 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_ZOOM
                    " ожидает 3 аргумента: center_x (0.0-1.0), "
                    "center_y (0.0-1.0), amount (>0.0)\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Проверяем, что аргументы - числа с плавающей
            // точкой
            if (!is_float(argv[i + 1]) ||
                !is_float(argv[i + 2]) ||
                !is_float(argv[i + 3])) {
                fprintf(
                    stderr,
                    "[Error] " IC_ARGV_FILTER_ZOOM
                    " ожидает числа с плавающей точкой\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            float center_x = atof(argv[i + 1]);
            float center_y = atof(argv[i + 2]);
            float amount = atof(argv[i + 3]);

            // Проверка диапазонов
            if (center_x < 0.0f || center_x > 1.0f ||
                center_y < 0.0f || center_y > 1.0f ||
                amount <= 0.0f) {
                fprintf(
                    stderr,
                    "[Error] center_x, center_y должны быть в "
                    "[0.0, 1.0], amount должен быть > 0.0\n"
                );
                return IC_ARGS_ASSISTANT_ERROR;
            }

            // Сохраняем параметры
            int params[3] = { (int)(center_x * 1000),
                              (int)(center_y * 1000),
                              (int)(amount * 1000) };

            Filter* zoom_filter =
                create_filter(ARGV_TYPE_FILTER_ZOOM, 3, params);

            // Добавляем в список
            if (*head == NULL) {
                *head = zoom_filter;
            } else {
                Filter* current = *head;
                while (current->next) {
                    current = current->next;
                }
                current->next = zoom_filter;
            }

            i += 3; // Пропускаем параметры

        } else {
            fprintf(
                stderr,
                "[Error] Неизвестный фильтр %s\n",
                argv[i]
            );
            return IC_ARGS_ASSISTANT_ERROR;
        }
    }

    return 0;
}

// Вспомогательная функция для освобождения списка фильтров
void free_filter_list(Filter* head) {
    while (head) {
        Filter* next = head->next;
        free(head);
        head = next;
    }
}

void printhelp() {
    FILE* help_file = fopen(AUX_DIR SLASH HELP_MESSAGE, "r");
    if (!help_file) {
        // Если файл не найден, выводим встроенный help
        printf("ImageCraft - BMP Image Processor\n");
        printf(
            "Использование: imagecraft <input.bmp> "
            "[output.bmpG]\n"
        );
        printf(
            "Используйте " IC_ARGV_HELP
            " для большей информации\n"
        );
        return;
    }

    // Читаем и выводим файл построчно
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), help_file)) {
        printf("%s", buffer);
    }

    fclose(help_file);
}

void printversion() {
    FILE* version_info =
        fopen(AUX_DIR SLASH VERSION_MESSAGE, "r");
    if (!version_info) {
        // Если файл не найден, выводим встроенный version
        printf("ImageCraft - BMP Image Processor\n");
        printf("Still in maintainance");
        return;
    }

    // Читаем и выводим файл построчно
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), version_info)) {
        printf("%s", buffer);
    }

    fclose(version_info);
}
