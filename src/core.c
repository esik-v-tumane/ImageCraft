#include "core.h"
#include "args_parser.h"
#include "bmp.h"

int imagecraft(int argc, char **argv) {
    char *ifile = NULL, *ofile = NULL;
    parse_args(argc, argv, &ifile, &ofile);
    if (!bmp_is_valid_24bit(ifile)) { // TODO: При argc = 1 всё равно выполняется функция
        perror("[Error] Not a valid 24-bit BMP file\n");
        return 1;
    }
    BMPImage *image = bmp_load(ifile);
    bmp_print_info(image);
    bmp_free(image);
}
