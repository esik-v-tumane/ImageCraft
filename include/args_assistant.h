#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

#include "filters.h"

int parse_args(
    int argc,
    char** argv,
    char** ifile,
    char** ofile,
    Filter** head
);

void printhelp();

void printversion();

#endif // !ARGS_PARSER_H
