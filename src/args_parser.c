#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"
#include "defines.h"
#include "filters.h"

int parse_args(
    int argc,
    char** argv,
    char** ifile,
    char** ofile,
    Filter* head
) {

    if (argc >= 2)
        *ifile = argv[1];
    if (argc >= 3)
        *ofile = argv[2];
    if (argc <= 3)
        return 0;

    printf("\n%s!!!!!!!!!!\n", *ofile);

    Filter* this = head;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], IC_FILTER_CROP) == 0) {
            //  -crop width height
            this->param_count = 2;

            if (i + 2 < argc) {
                this->params[INDEX_WIDTH] = atoi(argv[i + 1]);
                this->params[INDEX_HEIGHT] = atoi(argv[i + 2]);

                // Размеры > 0?
                if (this->params[INDEX_WIDTH] <= 0 ||
                    this->params[INDEX_HEIGHT] <= 0) {
                    fprintf(
                        stderr,
                        "[Error]: Размеры для обрезки должны "
                        "быть неотрицательными\n"
                    );
                    return 1;
                }

                printf(
                    "Обрезка: %d x %d\n",
                    this->params[INDEX_WIDTH],
                    this->params[INDEX_HEIGHT]
                );
                i += this->param_count; // Переход к следующему
                                        // фильтру
            } else {
                fprintf(
                    stderr,
                    "[Error]: -crop ожидает 2 аргмента: width, "
                    "height.\n"
                );
                return 1;
            }
        }
    }

    return 0;
}
