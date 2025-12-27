#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"
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

    // Первый аргумент - входной файл
    *ifile = argv[1];

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
        if (strcmp(argv[i], IC_FILTER_CROP) == 0) {
            // Проверяем, что есть достаточно аргументов
            if (i + 2 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_FILTER_CROP
                    " ожидает 2 аргумента: width, "
                    "height.\n"
                );
                // Освобождаем уже созданные фильтры при ошибке
                return 1;
            }

            // Проверяем, что аргументы - целые числа
            if (!is_integer(argv[i + 1]) ||
                !is_integer(argv[i + 2])) {
                fprintf(
                    stderr,
                    "[Error] " IC_FILTER_CROP
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
                return 1;
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
                return 1;
            }

            // Создаем фильтр обрезки
            int params[2] = { width, height };
            Filter* crop_filter =
                create_filter(FILTER_TYPE_CROP, 2, params);

            if (!crop_filter) {
                fprintf(
                    stderr,
                    "[Error] Не удалось выделить память для "
                    "фильтра\n"
                );
                return 1;
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

        } else if (strcmp(argv[i], IC_FILTER_GS) == 0) {
            // Фильтр grayscale без параметров
            Filter* gs_filter =
                create_filter(FILTER_TYPE_GS, 0, NULL);

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

        } else if (strcmp(argv[i], IC_FILTER_NEG) == 0) {
            // Фильтр negative без параметров
            Filter* neg_filter =
                create_filter(FILTER_TYPE_NEG, 0, NULL);

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

        } else if (strcmp(argv[i], IC_FILTER_SHARP) == 0) {
            // Фильтр sharpening без параметров
            Filter* sharp_filter =
                create_filter(FILTER_TYPE_SHARP, 0, NULL);

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

        } else if (strcmp(argv[i], IC_FILTER_EDGE) == 0) {
            // Фильтр edge detection с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_FILTER_EDGE
                    " ожидает 1 аргумент: "
                    "threshold\n"
                );
                return 1;
            }

            float threshold = atof(argv[i + 1]);
            // Сохраняем threshold как целое (умножаем на 1000
            // для точности)
            int threshold_int = (int)(threshold * 1000);
            int params[1] = { threshold_int };
            Filter* edge_filter =
                create_filter(FILTER_TYPE_EDGE, 1, params);

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

        } else if (strcmp(argv[i], IC_FILTER_BLUR) == 0) {
            // Фильтр blur с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_FILTER_BLUR
                    " ожидает 1 аргумент: sigma\n"
                );
                return 1;
            }

            float sigma = atof(argv[i + 1]);
            // Сохраняем sigma как целое (умножаем на 1000 для
            // точности)
            int sigma_int = (int)(sigma * 1000);
            int params[1] = { sigma_int };
            Filter* blur_filter =
                create_filter(FILTER_TYPE_BLUR, 1, params);

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

        } else if (strcmp(argv[i], IC_FILTER_MED) == 0) {
            // Фильтр median с одним параметром
            if (i + 1 >= argc) {
                fprintf(
                    stderr,
                    "[Error] " IC_FILTER_MED
                    " ожидает 1 аргумент: window\n"
                );
                return 1;
            }

            int window = atoi(argv[i + 1]);
            if (window <= 0 || window % 2 == 0) {
                fprintf(
                    stderr,
                    "[Error] window должен быть положительным "
                    "нечетным числом\n"
                );
                return 1;
            }

            int params[1] = { window };
            Filter* med_filter =
                create_filter(FILTER_TYPE_MED, 1, params);

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

        } else {
            fprintf(
                stderr,
                "[Error] Неизвестный фильтр: %s\n",
                argv[i]
            );
            return 1;
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
