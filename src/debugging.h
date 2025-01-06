#include <stdio.h>

#define DBG_INFO(fmt, ...) fprintf(stderr, "[INFO] [%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);
