#ifndef IC_FILTERS
#define IC_FILTERS

#include "bmp.h"
#include <stdint.h>

#define MAX_FILTER_PARAMS 3

// Наименования фильтров
#define IC_ARGV_FILTER_CROP "-crop"
#define IC_ARGV_FILTER_GS "-gs"
#define IC_ARGV_FILTER_NEG "-neg"
#define IC_ARGV_FILTER_SHARP "-sharp"
#define IC_ARGV_FILTER_EDGE "-edge"
#define IC_ARGV_FILTER_BLUR "-blur"
#define IC_ARGV_FILTER_MED "-med"
#define IC_ARGV_FILTER_VIGNETTE "-vignette"
#define IC_ARGV_FILTER_ZOOM "-zoomblur"

// Хеши аргументов (порядок важен!)
// TODO: Использовать настоящие хеш-функции
#define ARGV_TYPE_INFO 0
#define ARGV_TYPE_FILTER_CROP 1
#define ARGV_TYPE_FILTER_GS 2
#define ARGV_TYPE_FILTER_NEG 3
#define ARGV_TYPE_FILTER_SHARP 4
#define ARGV_TYPE_FILTER_EDGE 5
#define ARGV_TYPE_FILTER_BLUR 6
#define ARGV_TYPE_FILTER_MED 7
#define ARGV_TYPE_FILTER_VIGNETTE 8
#define ARGV_TYPE_FILTER_ZOOM 9

typedef struct _Filter {
    int type;
    int param_count;
    int params[MAX_FILTER_PARAMS];
    struct _Filter* next;
} Filter;

// Функции для работы с фильтрами
void free_filter_list(Filter* head);

// Функции фильтров
BMPImage* filter_crop(BMPImage* image, int width, int height);
int filter_grayscale(BMPImage* image);
int filter_negative(BMPImage* image);
int filter_sharpening(BMPImage* image);
BMPImage*
filter_edge_detection(BMPImage* image, float threshold);
BMPImage* filter_gaussian_blur(BMPImage* image, float sigma);
BMPImage* filter_median(BMPImage* image, int window);
int filter_vignette(
    BMPImage* image,
    float intensity,
    float radius
);
BMPImage* filter_zoom_blur(
    BMPImage* image,
    float center_x_ratio,
    float center_y_ratio,
    float amount
);

// Основная функция применения цепочки фильтров
int apply_filters(BMPImage** image, Filter* filter_list);

// Вспомогательные функции
float rgb_to_grayscale(uint8_t r, uint8_t g, uint8_t b);
uint8_t clamp_float_to_uint8(float value);

#endif // !IC_FILTERS
