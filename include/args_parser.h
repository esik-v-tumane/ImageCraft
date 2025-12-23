#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

#include "filters.h"
void printhelp();

int parse_args(
    int argc,
    char** argv,
    char** ifile,
    char** ofile,
    Filter* head
);

#endif // !ARGS_PARSER_H
