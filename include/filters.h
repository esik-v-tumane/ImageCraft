#ifndef IC_FILTERS
#define IC_FILTERS

#define MAX_FILTER_PARAMS 3

// Filters names
#define IC_FILTER_CROP "-crop"

typedef struct _Filter {
    int type;
    int param_count;
    int params[MAX_FILTER_PARAMS];
    struct _Filter* next;
} Filter;

#endif // !IC_FILTERS
