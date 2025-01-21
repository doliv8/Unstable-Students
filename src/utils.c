#include "utils.h"

int get_int() {
	printf("> ");
	int val;
	while (scanf(" %d", &val) != 1) getchar();
	return val;
}

void* malloc_checked(size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%lu)", __func__, size);
#endif
	void* ptr = malloc(size);
	if (!ptr) {
		fprintf(stderr, "Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	return ptr;
}

void* calloc_checked(size_t nmemb, size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%lu, %lu)", __func__, nmemb, size);
#endif
	void* ptr = calloc(nmemb, size);
	if (!ptr) {
		fprintf(stderr, "Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	return ptr;
}

void* realloc_checked(void* ptr, size_t size) {
#ifdef DEBUG
	DBG_INFO("called %s(%p, %lu)", __func__, ptr, size);
#endif
	void* new_ptr = realloc(ptr, size);
	if (!new_ptr) {
		fprintf(stderr, "Memory reallocation failed!\n");
		exit(EXIT_FAILURE);
	}
	return new_ptr;
}

void free_wrap(void* ptr) {
#ifdef DEBUG
	DBG_INFO("called %s(%p)", __func__, ptr);
#endif
	free(ptr);
}

int rand_int(int min, int max) {
	return rand() % (max+1) + min;
}

int read_int(FILE* fp) {
	int val;
	if (fscanf(fp, " %d", &val) != 1) {
		fprintf(stderr, "Error occurred while reading an integer from file stream!");
		exit(EXIT_FAILURE);
	}
	return val;
}

void init_wrapped(wrapped_textT* wrapped, char* text, int max_width) {
	int text_len = strlen(text);

	// initialize fields
	wrapped->text = malloc_checked(text_len+1);
	strncpy(wrapped->text, text, text_len+1);
	wrapped->n_lines = 1;
	wrapped->lines = calloc_checked(1, sizeof(char*));
	wrapped->lines[0] = wrapped->text;
	wrapped->lengths = calloc_checked(1, sizeof(int));
	wrapped->lengths[0] = text_len;

	char* last_space = wrapped->text;
	for (size_t pos = 0, line_len = 0; pos < text_len; pos++, line_len++) {
		if (wrapped->text[pos] != ' ') continue; // only interested in spaces.
		if (line_len >= max_width) {
			// split line and add second part to lines array
			*last_space = '\0'; // terminate this line's string
			wrapped->lines = realloc_checked(wrapped->lines, (wrapped->n_lines+1)*sizeof(char*));
			wrapped->lines[wrapped->n_lines] = last_space+1;

			wrapped->lengths = realloc_checked(wrapped->lengths, (wrapped->n_lines+1)*sizeof(int));
			wrapped->lengths[wrapped->n_lines+1] = wrapped->lengths[wrapped->n_lines] - line_len; // set next line's length
			wrapped->lengths[wrapped->n_lines] = line_len; // save this line's length

			wrapped->n_lines++;

			// starting a new line
			line_len = 0;
		}
		last_space = &wrapped->text[pos];
	}
}

void clear_wrapped(wrapped_textT* wrapped) {
	free(wrapped->lines);
	free(wrapped->lengths);
	free(wrapped->text);
}