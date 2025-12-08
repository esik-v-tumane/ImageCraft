#include <stdio.h>

#include "help.h"

void printhelp(void) {
    FILE* help_file = fopen("aux/" HELP_MESSAGE, "r");
    if (!help_file) {
        // Если файл не найден, выводим встроенный help
        printf("ImageCraft - BMP Image Processor\n");
        printf("Usage: imagecraft <input.bmp> [output.bmp]\n");
        printf("Use --help for more information\n"); // TODO: Реализовать --help
        return;
    }

    // Читаем и выводим файл построчно
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), help_file)) {
        printf("%s", buffer);
    }

    fclose(help_file);
}
