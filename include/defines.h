#ifndef IC_DEFINES
#define IC_DEFINES

// Пути и названия файлов по умолчанию
#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

#define AUX_DIR "aux"
#define SAVE_DIR ""
#define OUTFILE "output.bmp"

// Внутренние программные файлы
#define VERSION_MESSAGE ".version"
#define HELP_MESSAGE "help"

// Бинарные коды
#define INDEX_WIDTH 0
#define INDEX_HEIGHT 1 - INDEX_WIDTH

// Аргументы командной строки (кроме фильтров)
#define IC_ARGV_HELP "-help"
#define IC_ARGV_VERSION "-version"
#define IC_ARGV_INFO "-info"

// Корректные коды возврата
#define ALL_OK 0b0

// Ошибки работы с файлами
#define IC_ERROR_OPENING_FILE 0b1

// Ошибки чтения BMP
#define IC_BMP_ERROR_NULL_FILENAME 0b1001
#define IC_BMP_ERROR_INVALID_SIGNATURE 0b1011
#define IC_BMP_ERROR_INVALID_DIB 0b101
#define IC_BMP_ERROR_INVALID_BPP 0b1111

// Ошибки записи BMP
#define IC_BMP_ERROR_SAVING_FILE 0b10001
#define IC_BMP_ERROR_WRITING_HEADER 0b10011
#define IC_BMP_ERROR_ALLOCATING_BUFFER 0b10101
#define IC_BMP_ERROR_WRITING_ROW 0b10111

// Ошибки ядра
#define IC_ERROR_KERNEL_FAILURE 0b100001;

// Результаты обработки аргументов
#define IC_ARGS_ASSISTANT_OK 0b00
#define IC_ARGS_ASSISTANT_HELP 0b01
#define IC_ARGS_ASSISTANT_VERSION 0b10
#define IC_ARGS_ASSISTANT_INFO 0b11
#define IC_ARGS_ASSISTANT_ERROR 0b100

// Сообщения о сообщениях об ошибках
#define IC_MESSAGE_ERROR_OPENING_FILE "IC_ERROR_OPENING_FILE"
#define IC_MESSAGE_BMP_ERROR_NULL_FILENAME                      \
    "IC_BMP_ERROR_NULL_FILENAME"
#define IC_MESSAGE_BMP_ERROR_INVALID_SIGNATURE                  \
    "IC_BMP_ERROR_INVALID_SIGNATURE"
#define IC_MESSAGE_BMP_ERROR_INVALID_DIB                        \
    "IC_BMP_ERROR_INVALID_DIB"
#define IC_MESSAGE_BMP_ERROR_INVALID_BPP                        \
    "IC_BMP_ERROR_INVALID_BPP"

#endif // !IC_DEFINES
#define IC_DEFINES
