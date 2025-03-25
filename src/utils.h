#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "structs.h"

// macros to perform string conversion at compile time
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define MIN(a, b) (a < b ? a : b)

int get_int(void);

void *malloc_checked(size_t size);
void *calloc_checked(size_t nmemb, size_t size);
void *realloc_checked(void *ptr, size_t size);
void free_wrap(const void *ptr);

char *strdup_checked(const char *str);

int rand_int(int min, int max);

bool ask_choice(void);

#endif // UTILS_H