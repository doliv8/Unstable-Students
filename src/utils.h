#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "types.h"
#include "structs.h"

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define MIN(a, b) (a < b ? a : b)

int get_int();

void *malloc_checked(size_t size);
void *calloc_checked(size_t nmemb, size_t size);
void *realloc_checked(void *ptr, size_t size);
void free_wrap(const void *ptr);

char *strdup_checked(const char *str);
int asprintf_s(char **strp, const char *fmt, const char *s0);
int asprintf_ss(char **strp, const char *fmt, const char *s0, const char *s1);
int asprintf_sss(char **strp, const char *fmt, const char *s0, const char *s1, const char *s2);
int asprintf_d(char **strp, const char *fmt, int d0);
char *center_boxed_string(const char *str, int str_len, const char *border, int width);

int rand_int(int min, int max);

bool ask_choice();

void init_multiline(multiline_textT *multiline);
void clear_multiline(multiline_textT *multiline);
void clear_freeable_multiline(freeable_multiline_textT *freeable_multiline);
void multiline_addline(multiline_textT *multiline, const char *line);
void multiline_addline_with_len(multiline_textT *multiline, const char *line, int len);
void print_centered_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width);
void print_centered_boxed_multiline(multiline_textT *multiline, const char *border, int width);

void init_wrapped(wrapped_textT *wrapped, const char *text, int max_width);
void clear_wrapped(wrapped_textT *wrapped);

void init_multiline_container(multiline_containerT *container);
void clear_multiline_container(multiline_containerT *container);
void container_addmultiline(multiline_containerT *container, multiline_textT *multiline);

#endif // UTILS_H