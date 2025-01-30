#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
		fputs("Memory allocation failed!", stderr);
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
		fputs("Memory allocation failed!", stderr);
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
		fputs("Memory reallocation failed!", stderr);
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
		fputs("Error occurred while reading an integer from file stream!", stderr);
		exit(EXIT_FAILURE);
	}
	return val;
}

char* strdup_checked(const char* str) {
	int len = strlen(str)+1;
	char* copy = malloc_checked(len);
	strncpy(copy, str, len);
	return copy;
}

int vget_formatted_length(const char* fmt, va_list args) {
	int length = vsnprintf(NULL, 0, fmt, args);
	return length;
}

int get_formatted_length(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int length = vget_formatted_length(fmt, ap);
	va_end(ap);
	return length;
}

int asprintf_checked(char** strp, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int length = vget_formatted_length(fmt, ap);
	va_end(ap);
	*strp = malloc_checked(length+1);
	va_start(ap, fmt);
	int result = vsnprintf(*strp, length+1, fmt, ap);
	va_end(ap);
	if (result < 0) {
		fputs("Error occurred while formatting a dynamically allocated string!", stderr);
		exit(EXIT_FAILURE);
	}
	return result;
}

void init_multiline(multiline_textT* multiline) {
	multiline->n_lines = 0;
	multiline->lines = NULL;
	multiline->lengths = NULL;
}

void clear_multiline(multiline_textT* multiline) {
	free(multiline->lines);
	free(multiline->lengths);
}

void clear_freeable_multiline(freeable_multiline_textT* freeable_multiline) {
	for (int i = 0; i < freeable_multiline->n_lines; i++)
		free(freeable_multiline->lines[i]);
	clear_multiline(freeable_multiline);
}

void multiline_addline(multiline_textT* multiline, const char* line) {
	// increase arrays sizes
	multiline->n_lines++;
	multiline->lines = realloc_checked(multiline->lines, multiline->n_lines*sizeof(char*));
	multiline->lengths = realloc_checked(multiline->lengths, multiline->n_lines*sizeof(int));
	// add actual line
	multiline->lines[multiline->n_lines-1] = line;
	multiline->lengths[multiline->n_lines-1] = strlen(line);
}

void multiline_addline_with_len(multiline_textT* multiline, const char* line, int len) {
	multiline_addline(multiline, line);
	multiline->lengths[multiline->n_lines-1] = len;
}

void init_wrapped(wrapped_textT* wrapped, char* text, int max_width) {
	int text_len = strlen(text);
	init_multiline(&wrapped->multiline);

	// initialize fields
	wrapped->text = strdup_checked(text);
	multiline_addline(&wrapped->multiline, wrapped->text);

	char* last_space = wrapped->text;
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

char *center_lr_boxed_string(const char* str, int str_len, const char* l_border, const char* r_border, int width) {
	int padding = width - str_len;
	int l_padding = padding / 2;
	int r_padding = padding - l_padding;
	char *formatted;
	asprintf_checked(&formatted, "%s%*s%s%*s%s", l_border, l_padding, "", str, r_padding, "", r_border);
	return formatted;
}

char *center_boxed_string(const char* str, int str_len, const char* border, int width) {
	return center_lr_boxed_string(str, str_len, border, border, width);
}

void print_centered_lr_boxed_string(const char* str, int str_len, const char* l_border, const char* r_border, int width) {
	char *formatted = center_lr_boxed_string(str, str_len, l_border, r_border, width);
	printf("%s\n", formatted);
	free(formatted);
}

void print_centered_boxed_multiline(multiline_textT* multiline, const char* border, int width) {
	for (int i = 0; i < multiline->n_lines; i++)
		print_centered_lr_boxed_string(multiline->lines[i], multiline->lengths[i], border, border, width);
}

void clear_wrapped(wrapped_textT* wrapped) {
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

const char* quandoT_str(quandoT quando) {
	static const char *mapping[] = {
		[SUBITO] = "Subito",
		[INIZIO] = "Inizio",
		[FINE] = "Fine",
		[MAI] = "Mai",
		[SEMPRE] = "Sempre"
	};
	return mapping[quando];
}

const char* target_giocatoriT_str(target_giocatoriT target) {
	static const char *mapping[] = {
		[IO] = "Io",
		[TU] = "Tu",
		[VOI] = "Voi",
		[TUTTI] = "Tutti"
	};
	return mapping[target];
}

const char* tipo_cartaT_str(tipo_cartaT tipo) {
	static const char *mapping[] = {
		[ALL] = "All",
		[STUDENTE] = "Studente",
		[MATRICOLA] = "Matricola",
		[STUDENTE_SEMPLICE] = "Studente semplice",
		[LAUREANDO] = "Laureando",
		[BONUS] = "Bonus",
		[MALUS] = "Malus",
		[MAGIA] = "Magia",
		[ISTANTANEA] = "Istantanea"
	};
	return mapping[tipo];
}

const char* azioneT_str(azioneT azione) {
	static const char *mapping[] = {
		[GIOCA] = "Gioca",
		[SCARTA] = "Scarta",
		[ELIMINA] = "Elimina",
		[RUBA] = "Ruba",
		[PESCA] = "Pesca",
		[PRENDI] = "Prendi",
		[BLOCCA] = "Blocca",
		[SCAMBIA] = "Scambia",
		[MOSTRA] = "Mostra",
		[IMPEDIRE] = "Impedire",
		[INGEGNERE] = "Ingegnere"
	};
	return mapping[azione];
}