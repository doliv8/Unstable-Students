#include "utils.h"

int get_int() {
	printf("> ");
	int val;
	while (scanf(" %d", &val) != 1) ;
	return val;
}

void* malloc_checked(size_t size) {
	void* ptr = malloc(size);
	if (!ptr) {
		fprintf(stderr, "Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	return ptr;
}

void* calloc_checked(size_t nmemb, size_t size) {
#ifdef DEBUG
	DBG_INFO("called calloc(%lu, %lu)", nmemb, size);
#endif
	void* ptr = calloc(nmemb, size);
	if (!ptr) {
		fprintf(stderr, "Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	return ptr;
}