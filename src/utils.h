#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debugging.h"
#include "types.h"
#include "structs.h"

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

int get_int();

void* malloc_checked(size_t size);
void* calloc_checked(size_t nmemb, size_t size);
void* realloc_checked(void* ptr, size_t size);
void free_wrap(void* ptr);

char* strdup_checked(const char* str);

// generates a random integer in the range [min, max]
int rand_int(int min, int max);

int read_int(FILE* fp);

void init_multiline(multiline_textT* multiline);
void clear_multiline(multiline_textT* multiline);
void clear_freeable_multiline(freeable_multiline_textT* freeable_multiline);
void multiline_addline(multiline_textT* multiline, const char* line);
void multiline_addline_with_len(multiline_textT* multiline, const char* line, int len);
void print_centered_boxed_string(const char* str, int str_len, const char* border, int width);
void print_centered_boxed_multiline(multiline_textT* multiline, const char* border, int width);
void init_wrapped(wrapped_textT* wrapped, char* text, int max_width);
void clear_wrapped(wrapped_textT* wrapped);


// TODO: move away in proper file
const char* quandoT_str(quandoT quando);
const char* target_giocatoriT_str(target_giocatoriT target);
const char* tipo_cartaT_str(tipo_cartaT tipo);
const char* azioneT_str(azioneT azione);

#endif