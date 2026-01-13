#ifndef IC_PATHS
#define IC_PATHS

char* ic_strdup(const char* s);

int create_output_directory_recursive(const char* path);

char* get_directory_from_path(const char* filepath);

#endif // !IC_PATHS
