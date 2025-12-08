#include "core.h"
#include "args_parser.h"
#include "bmp.h"
#include "defines.h"
#include "help.h"

int imagecraft(int argc, char** argv) {
    char *ifile = NULL, *ofile = NULL;
    parse_args(argc, argv, &ifile, &ofile);

    if (argc == 1) {
        printhelp();
        return 0;
    }

    int error;
    if (((error = bmp_is_valid_24bit(ifile)) & 1) == 1) {
        if (error == IC_BMP_ERROR_OPENING_FILE)
            fprintf(stderr, "[Error] Не получилось прочитать файл '%s'\n", ifile);
        else // иначе проблема в структуре BMP
        {
            char* error_code;
            fprintf(stderr, "[Error] Некорректный файл BMP (код ошибки ");

            // Определение кода ошибки
            if (error == IC_BMP_ERROR_NULL_FILENAME)
                error_code = IC_MESSAGE_BMP_ERROR_NULL_FILENAME;
            else if (error == IC_BMP_ERROR_INVALID_SIGNATURE)
                error_code = IC_MESSAGE_BMP_ERROR_INVALID_SIGNATURE;
            else if (error == IC_BMP_ERROR_INVALID_BPP)
                error_code = IC_MESSAGE_BMP_ERROR_INVALID_BPP;
            else if (error == IC_BMP_ERROR_INVALID_DIB)
                error_code = IC_MESSAGE_BMP_ERROR_INVALID_DIB;

            fprintf(stderr, "%s)\n", error_code);
        }
        return error;
    }
    BMPImage* image = bmp_load(ifile);
    bmp_print_info(image);
    bmp_free(image);
    return ALL_OK;
}
