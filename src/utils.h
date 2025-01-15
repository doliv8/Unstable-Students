#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "debugging.h"

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

int get_int();
void* malloc_checked(size_t size);
void* calloc_checked(size_t nmemb, size_t size);
void free_wrap(void* ptr);

// generates a random integer in the range [min, max]
int rand_int(int min, int max);

int read_int(FILE* fp);


#endif