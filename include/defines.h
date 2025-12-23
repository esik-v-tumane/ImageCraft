#ifndef IC_DEFINES
#define IC_DEFINES

// Пути и названия файлов по умолчанию
#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

#define SAVE_DIR "tmp"
#define OUTFILE "output.bmp"

#define INDEX_WIDTH 0
#define INDEX_HEIGHT 1 - INDEX_WIDTH

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
