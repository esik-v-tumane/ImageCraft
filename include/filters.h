#ifndef IC_FILTERS
#define IC_FILTERS

#include "bmp.h"
#include <stdint.h>

#define MAX_FILTER_PARAMS 3

// Наименования фильтров
#define IC_FILTER_CROP "-crop"
#define IC_FILTER_GS "-gs"
#define IC_FILTER_NEG "-neg"
#define IC_FILTER_SHARP "-sharp"
#define IC_FILTER_EDGE "-edge"
#define IC_FILTER_BLUR "-blur"
#define IC_FILTER_MED "-med"

// Хеши фильтров (порядок важен!)
// TODO: Использовать настоящие хеш-функции
#define FILTER_TYPE_CROP 1
#define FILTER_TYPE_GS 2
#define FILTER_TYPE_NEG 3
#define FILTER_TYPE_SHARP 4
#define FILTER_TYPE_EDGE 5
#define FILTER_TYPE_BLUR 6
#define FILTER_TYPE_MED 7

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

// Основная функция применения цепочки фильтров
int apply_filters(BMPImage** image, Filter* filter_list);

// Вспомогательные функции
float rgb_to_grayscale(uint8_t r, uint8_t g, uint8_t b);
uint8_t clamp_float_to_uint8(float value);

#endif // !IC_FILTERS
