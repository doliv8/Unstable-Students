#ifndef FORMAT_H
#define FORMAT_H

#include "types.h"
#include "structs.h"


int asprintf_s(char **strp, const char *fmt, const char *s0);
int asprintf_ss(char **strp, const char *fmt, const char *s0, const char *s1);
int asprintf_sss(char **strp, const char *fmt, const char *s0, const char *s1, const char *s2);
int asprintf_d(char **strp, const char *fmt, int d0);
char *center_boxed_string(const char *str, int str_len, const char *border, int width);

void init_multiline(multiline_textT *multiline);
void clear_multiline(multiline_textT *multiline);
void clear_freeable_multiline(freeable_multiline_textT *freeable_multiline);
void multiline_addline(multiline_textT *multiline, const char *line);
void multiline_addline_with_len(multiline_textT *multiline, const char *line, int len);
void print_centered_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width);
void print_centered_boxed_multiline(multiline_textT *multiline, const char *border, int width);

void wrap_text(wrapped_textT *wrapped, const char *text, int max_width);
void clear_wrapped(wrapped_textT *wrapped);

#endif // FORMAT_H