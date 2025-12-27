#include "paths.h"

#include <errno.h>
#include <libgen.h> // Для dirname()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char* ic_strdup(const char* s) {
    size_t size = strlen(s) + 1; // + '\0'
    char* p = malloc(size);

    if (p) {
        memcpy(p, s, size);
    }
    return p;
}

// Рекурсивное создание директорий (если их нет)
int create_output_directory_recursive(const char* path) {
    char tmp[1024];
    char* p = NULL;
    size_t len;

    // Копируем путь
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    // Убираем завершающий слеш
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = '\0';
    }

    // Создаем директории по пути
    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0'; // Временно обрезаем строку

            // Создаем директорию (если еще не существует)
#ifdef _WIN32
            if (mkdir(tmp) != 0 && errno != EEXIST) {
#else
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
#endif
                return -1;
            }

            *p = '/'; // Восстанавливаем разделитель
        }
    }

    // Создаем конечную директорию
#ifdef _WIN32
    if (mkdir(tmp) != 0 && errno != EEXIST) {
#else
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
#endif
        return -1;
    }

    return 0;
}

// Функция для извлечения директории из пути к файлу
// Возвращает NULL, если путь не содержит директории
char* get_directory_from_path(const char* filepath) {
    static char dir[1024];
    char* last_slash;

    if (!filepath) {
        return NULL;
    }

    // Копируем путь
    strncpy(dir, filepath, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';

    // Ищем последний разделитель
    last_slash = strrchr(dir, '/');
#ifdef _WIN32
    char* last_backslash = strrchr(dir, '\\');
    if (last_backslash && last_backslash > last_slash) {
        last_slash = last_backslash;
    }
#endif

    if (last_slash) {
        // Если разделитель найден и это не начало пути
        if (last_slash != dir || *(last_slash + 1) != '\0') {
            *last_slash = '\0';
            return dir;
        }
    }

    // Если разделителя нет или это просто имя файла
    return NULL;
}
