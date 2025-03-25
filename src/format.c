#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "format.h"
#include "utils.h"

/**
 * @brief call this when an error in dynamic formatting happens. does not return.
 * 
 */
void formatting_failed(void) {
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
	int length, result;
	if (fmt == NULL)
		formatting_failed();
	length = snprintf(NULL, 0, fmt, d0);
	*strp = malloc_checked(length+1);
	result = snprintf(*strp, length+1, fmt, d0);
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
	int length, result;
	if (fmt == NULL)
		formatting_failed();
	length = snprintf(NULL, 0, fmt, s0);
	*strp = malloc_checked(length+1);
	result = snprintf(*strp, length+1, fmt, s0);
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
	int length, result;
	if (fmt == NULL)
		formatting_failed();
	length = snprintf(NULL, 0, fmt, s0, s1);
	*strp = malloc_checked(length+1);
	result = snprintf(*strp, length+1, fmt, s0, s1);
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
	int length, result;
	if (fmt == NULL)
		formatting_failed();
	length = snprintf(NULL, 0, fmt, s0, s1, s2);
	*strp = malloc_checked(length+1);
	result = snprintf(*strp, length+1, fmt, s0, s1, s2);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief dynamically formats a string with four string parameter
 * 
 * @param strp pointer to formatted string
 * @param fmt format string
 * @param s0 param 0 (string)
 * @param s1 param 1 (string)
 * @param s2 param 2 (string)
 * @param s3 param 3 (string)
 * @return int length of formatted string
 */
int asprintf_ssss(char **strp, const char *fmt, const char *s0, const char *s1, const char *s2, const char *s3) {
	int length, result;
	if (fmt == NULL)
		formatting_failed();
	length = snprintf(NULL, 0, fmt, s0, s1, s2, s3);
	*strp = malloc_checked(length+1);
	result = snprintf(*strp, length+1, fmt, s0, s1, s2, s3);
	if (result < 0)
		formatting_failed();
	return result;
}

/**
 * @brief initialize multiline_textT structure internal fields, must always be called before using multiline_textT or freeable_multiline_textT
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
 * @brief append a new line to the multiline specifying its visual length (usedful for pretty printing)
 * 
 * @param multiline pointer to the multiline
 * @param line string to add
 * @param len visual length of the string
 */
void multiline_addline_with_len(multiline_textT *multiline, const char *line, int len) {
	multiline_addline(multiline, line);
	multiline->lengths[multiline->n_lines-1] = len;
}

/**
 * @brief Wrap a string into a wrapped_textT with a specified max width.
 * Works by creating a heap copy of the string and replacing spaces with NULL-terminators for signaling end of each line, thus creating
 * new substrings which are referenced in the wrapped's multiline.
 * 
 * @param wrapped pointer to the wrapped
 * @param text string to wrap
 * @param max_width maximum width to use in wrapping text
 */
void wrap_text(wrapped_textT *wrapped, const char *text, size_t max_width) {
	size_t text_len = strlen(text);
	init_multiline(&wrapped->multiline);

	// initialize fields
	wrapped->text = strdup_checked(text); // create a heap copy of text
	multiline_addline(&wrapped->multiline, wrapped->text); // add initial first line (a substring yet to be terminated in the right spot)

	char *last_space = wrapped->text;
	for (size_t pos = 0, line_len = 0, word_len = 0; pos <= text_len; pos++) { // iterate until the NULL-terminator (included)
		if (wrapped->text[pos] == ' ' || wrapped->text[pos] == '\0') { // only interested in spaces and terminator
			if (line_len+word_len >= max_width) { // check if this word doesn't fit in the current line withing the max_width
				// split line into two substrings first
				*last_space = '\0'; // terminate this line (first substring)

				wrapped->multiline.lengths[wrapped->multiline.n_lines-1] = line_len-1; // save this line's length (remove added space)

				multiline_addline(&wrapped->multiline, last_space+1); // add next line (second substring) to multiline (skipping the space)

				line_len = 0; // starting a new line
			}
			last_space = &wrapped->text[pos]; // update pointer to last encountered space (possible split point of the current line)
			line_len += word_len+1; // include space
			word_len = 0; // starting a new word
		} else
			word_len++; // just a normal characted of a word, increase word length
	}
}

/**
 * @brief free up memory allocated by wrapped_textT structure. must always be called after finished using the wrapped
 * 
 * @param wrapped pointer to wrapped
 */
void clear_wrapped(wrapped_textT *wrapped) {
	clear_multiline(&wrapped->multiline);
	free_wrap(wrapped->text);
}

/**
 * @brief centers a string in a specified width and adds left and right borders to it (not included in the width)
 * 
 * @param str string to be centered and boxed
 * @param str_len visual length of str
 * @param l_border left border string
 * @param r_border right border string
 * @param width width to center str into
 * @return char* formatted centered and boxed string
 */
char *center_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width) {
	int length, padding = width - str_len;
	int l_padding = padding / 2;
	int r_padding = padding - l_padding;
	char *formatted;

	// dynamically allocate the formatted string as asprintf would do
	length = snprintf(NULL, 0, "%s%*s%s%*s%s", l_border, l_padding, "", str, r_padding, "", r_border);
	formatted = malloc_checked(length+1);
	if (snprintf(formatted, length+1, "%s%*s%s%*s%s", l_border, l_padding, "", str, r_padding, "", r_border) < 0)
		formatting_failed();

	return formatted;
}

/**
 * @brief centers a string in a specified width and adds border to its left and right (not included in the width)
 * 
 * @param str string to be centered and boxed
 * @param str_len visual length of str
 * @param border border string
 * @param width width to center str into
 * @return char* formatted centered and boxed string
 */
char *center_boxed_string(const char *str, int str_len, const char *border, int width) {
	return center_lr_boxed_string(str, str_len, border, border, width);
}

/**
 * @brief prints a line with only the centered string in the specified width surrounded left and right by borders (not included in the width)
 * 
 * @param str string to be printed center and boxed
 * @param str_len visual length of str
 * @param l_border left border string
 * @param r_border right border string
 * @param width width to center str into
 */
void print_centered_lr_boxed_string(const char *str, int str_len, const char *l_border, const char *r_border, int width) {
	char *formatted = center_lr_boxed_string(str, str_len, l_border, r_border, width);
	puts(formatted);
	free_wrap(formatted);
}

/**
 * @brief prints each line of the multiline centered within the specified width and boxed around given border (not included in the width)
 * 
 * @param multiline pointer to the multiline
 * @param border border string
 * @param width width to center lines into
 */
void print_centered_boxed_multiline(multiline_textT *multiline, const char *border, int width) {
	for (int i = 0; i < multiline->n_lines; i++)
		print_centered_lr_boxed_string(multiline->lines[i], multiline->lengths[i], border, border, width);
}
