#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "utils.h"
#include "debugging.h"

/**
 * @brief prompts user to insert an integer from standard input, doesn't return until an integer is supplied
 * 
 * @return int inserted integer
 */
int get_int() {
	printf("> ");
	int val;
	while (scanf(" %d", &val) != 1)
		getchar();
	return val;
}

/**
 * @brief prompts user to make a yes or no choice
 * 
 * @return true if user chose yes
 * @return false if user chose no
 */
bool ask_choice() {
	char choice = 'n';
	printf("(y/N): ");
	scanf(" %c", &choice);
	return choice == 'y' || choice == 'Y';
}

/**
 * @brief wrap around malloc() function, shows debug trace if compiled with -DDEBUG
 * 
 * @param size wanted size
 * @return void* allocated block
 */
void *malloc_checked(size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%lu)", __func__, size);
#endif
	void *ptr = malloc(size);
	if (!ptr) {
		fputs("Memory allocation failed!", stderr);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

/**
 * @brief wrap around calloc() function, shows debug trace if compiled with -DDEBUG
 * 
 * @param nmemb number of elements
 * @param size size of each element
 * @return void* allocated block
 */
void *calloc_checked(size_t nmemb, size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%lu, %lu)", __func__, nmemb, size);
#endif
	void *ptr = calloc(nmemb, size);
	if (!ptr) {
		fputs("Memory allocation failed!", stderr);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

/**
 * @brief wrap around realloc() function, shows debug trace if compiled with -DDEBUG
 * 
 * @param ptr block to resize
 * @param size wanted size
 * @return void* new block
 */
void *realloc_checked(void *ptr, size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%p, %lu)", __func__, ptr, size);
#endif
	void *new_ptr = realloc(ptr, size);
	if (!new_ptr) {
		fputs("Memory reallocation failed!", stderr);
		exit(EXIT_FAILURE);
	}
	return new_ptr;
}

/**
 * @brief wrap around free() function, shows debug trace if compiled with -DDEBUG
 * 
 * @param ptr block to free
 */
void free_wrap(const void *ptr) {
#ifdef DEBUG
	DBG_INFO("called %s(%p)", __func__, ptr);
#endif
	free((void*)ptr);
}

/**
 * @brief generates random int in range [min, max]
 * 
 * @param min lower value (included)
 * @param max higher value (included)
 * @return int generated int
 */
int rand_int(int min, int max) {
	return rand() % (max-min+1) + min;
}

/**
 * @brief mimics functionality of strdup() function.
 * allocates a heap block to store a copy fo the provided string and returns a pointer to it.
 * 
 * @param str string to duplicate
 * @return char* pointer to the heap block containing the duplicated string
 */
char *strdup_checked(const char *str) {
	int len = strlen(str)+1;
	char *copy = malloc_checked(len);
	strncpy(copy, str, len);
	return copy;
}

/**
 * @brief call this when an error in dynamic formatting happens. does not return.
 * 
 */
void formatting_failed() {
	fputs("Error occurred while formatting a dynamically allocated string!", stderr);
	exit(EXIT_FAILURE);
}

/**
 * @brief dynamically formats a string with one integer parameter
 * 
 * @param strp pointer to formatted string
 * @param fmt format string
 * @param d0 param 0 (int)
 * @return int length of formatted string
 */
int asprintf_d(char **strp, const char *fmt, int d0) {
	int length = snprintf(NULL, 0, fmt, d0);
	*strp = malloc_checked(length+1);
	int result = snprintf(*strp, length+1, fmt, d0);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief dynamically formats a string with one string parameter
 * 
 * @param strp pointer to formatted string
 * @param fmt format string
 * @param s0 param 0 (string)
 * @return int length of formatted string
 */
int asprintf_s(char **strp, const char *fmt, const char *s0) {
	int length = snprintf(NULL, 0, fmt, s0);
	*strp = malloc_checked(length+1);
	int result = snprintf(*strp, length+1, fmt, s0);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief dynamically formats a string with two string parameter
 * 
 * @param strp pointer to formatted string
 * @param fmt format string
 * @param s0 param 0 (string)
 * @param s1 param 1 (string)
 * @return int length of formatted string
 */
int asprintf_ss(char **strp, const char *fmt, const char *s0, const char *s1) {
	int length = snprintf(NULL, 0, fmt, s0, s1);
	*strp = malloc_checked(length+1);
	int result = snprintf(*strp, length+1, fmt, s0, s1);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief dynamically formats a string with three string parameter
 * 
 * @param strp pointer to formatted string
 * @param fmt format string
 * @param s0 param 0 (string)
 * @param s1 param 1 (string)
 * @param s2 param 2 (string)
 * @return int length of formatted string
 */
int asprintf_sss(char **strp, const char *fmt, const char *s0, const char *s1, const char *s2) {
	int length = snprintf(NULL, 0, fmt, s0, s1, s2);
	*strp = malloc_checked(length+1);
	int result = snprintf(*strp, length+1, fmt, s0, s1, s2);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief initialize multiline_textT structure, must always be called before using the multiline
 * 
 * @param multiline pointer to the multiline
 */
void init_multiline(multiline_textT *multiline) {
	multiline->n_lines = 0;
	multiline->lines = NULL;
	multiline->lengths = NULL;
}

/**
 * @brief free up memory allocated by multiline_textT structure, must always be called after finished using the multiline
 * 
 * @param multiline pointer to the multiline
 */
void clear_multiline(multiline_textT *multiline) {
	free_wrap(multiline->lines);
	free_wrap(multiline->lengths);
}

/**
 * @brief free up memory allocated by freeable_multiline_textT structure (hence all its lines),
 * must always be called after finished using the freeable_multiline
 * 
 * @param freeable_multiline pointer to the freeable multiline
 */
void clear_freeable_multiline(freeable_multiline_textT *freeable_multiline) {
	for (int i = 0; i < freeable_multiline->n_lines; i++)
		free_wrap(freeable_multiline->lines[i]);
	clear_multiline(freeable_multiline);
}

/**
 * @brief append a new line to the multiline
 * 
 * @param multiline pointer to the multiline
 * @param line string to add
 */
void multiline_addline(multiline_textT *multiline, const char *line) {
	// increase arrays sizes
	multiline->n_lines++;
	// realloc arrays
	multiline->lines = realloc_checked(multiline->lines, multiline->n_lines*sizeof(char*));
	multiline->lengths = realloc_checked(multiline->lengths, multiline->n_lines*sizeof(int));
	// add actual line
	multiline->lines[multiline->n_lines-1] = line;
	multiline->lengths[multiline->n_lines-1] = strlen(line);
}

/**
 * @brief append a new line to the multiline specifying its visual length (used pretty printing)
 * 
 * @param multiline pointer to the multiline
 * @param line string to add
 * @param len visual length of the string
 */
void multiline_addline_with_len(multiline_textT *multiline, const char *line, int len) {
	multiline_addline(multiline, line);
	multiline->lengths[multiline->n_lines-1] = len;
}

void init_wrapped(wrapped_textT* wrapped, const char *text, int max_width) {
	int text_len = strlen(text);
	init_multiline(&wrapped->multiline);

	// initialize fields
	wrapped->text = strdup_checked(text);
	multiline_addline(&wrapped->multiline, wrapped->text);

	char *last_space = wrapped->text;
	for (size_t pos = 0, line_len = 0, word_len = 0; pos <= text_len; pos++) {
		if (wrapped->text[pos] == ' ' || wrapped->text[pos] == '\0') { // only interested in spaces and terminator
			if (line_len+word_len >= max_width) {
				// split line and add second part to lines array
				*last_space = '\0'; // terminate this line's string

				wrapped->multiline.lengths[wrapped->multiline.n_lines-1] = line_len-1; // save this line's length (remove added space)
				multiline_addline(&wrapped->multiline, last_space+1); // add next line to multiline container

				// starting a new line
				line_len = 0;
			}
			last_space = &wrapped->text[pos];
			line_len += word_len+1;
			word_len = 0;
		} else
			word_len++;
	}
}

char *center_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width) {
	int length, padding = width - str_len;
	int l_padding = padding / 2;
	int r_padding = padding - l_padding;
	char *formatted;

	length = snprintf(NULL, 0, "%s%*s%s%*s%s", l_border, l_padding, "", str, r_padding, "", r_border);
	formatted = malloc_checked(length+1);
	if (snprintf(formatted, length+1, "%s%*s%s%*s%s", l_border, l_padding, "", str, r_padding, "", r_border) < 0)
		formatting_failed();
	return formatted;
}

char *center_boxed_string(const char *str, int str_len, const char *border, int width) {
	return center_lr_boxed_string(str, str_len, border, border, width);
}

void print_centered_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width) {
	char *formatted = center_lr_boxed_string(str, str_len, l_border, r_border, width);
	printf("%s\n", formatted);
	free(formatted);
}

void print_centered_boxed_multiline(multiline_textT *multiline, const char *border, int width) {
	for (int i = 0; i < multiline->n_lines; i++)
		print_centered_lr_boxed_string(multiline->lines[i], multiline->lengths[i], border, border, width);
}

void clear_wrapped(wrapped_textT *wrapped) {
	clear_multiline(&wrapped->multiline);
	free_wrap(wrapped->text);
}

void init_multiline_container(multiline_containerT *container) {
	container->n_multilines = 0;
	container->multilines = NULL;
}

void clear_multiline_container(multiline_containerT *container) {
	free_wrap(container->multilines);
}

void container_addmultiline(multiline_containerT *container, multiline_textT *multiline) {
	container->n_multilines++;
	realloc_checked(container->multilines, container->n_multilines*sizeof(multiline_textT*));
	container->multilines[container->n_multilines-1] = multiline;
}