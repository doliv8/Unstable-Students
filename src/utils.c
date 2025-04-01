#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "constants.h"
#include "debugging.h"

/**
 * @brief prompts user to insert an integer from standard input, doesn't return until an integer is supplied
 * 
 * @return int inserted integer
 */
int get_int(void) {
	printf("> ");
	int val;
	while (scanf(" %d", &val) != ONE_ELEMENT)
		getchar();
	return val;
}

/**
 * @brief prompts user to make a yes or no choice
 * 
 * @return true if user chose yes
 * @return false if user chose no
 */
bool ask_choice(void) {
	char choice = 'n';
	printf("(" ANSI_GREEN "y" ANSI_RESET "/" ANSI_RED "N" ANSI_RESET "): ");
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
	size_t len = strlen(str)+1;
	char *copy = malloc_checked(len);
	strncpy(copy, str, len);
	return copy;
}