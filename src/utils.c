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

void free_wrap(void* ptr) {
#ifdef DEBUG
	DBG_INFO("called %s(%p)", __func__, ptr);
#endif
	free(ptr);
}

int rand_int(int min, int max) {
	return rand() % (max+1) + min;
}