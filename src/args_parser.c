#include <stdio.h>

#include "args_parser.h"

void parse_args(int argc, char** argv, char** ifile, char** ofile) {
    // printf("[Args Parser] %d argc\n", argc);
    switch (argc) {
    case 1:
        break;
    case 2:
        *ifile = argv[1];
        break;
    case 3:
        *ifile = argv[1];
        *ofile = argv[2];
        break;
    default:
        printf("Here are %d args\n", argc);
    }
}
