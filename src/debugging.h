/*
* only used for debug builds (compiling with -DDEBUG parameter)
*/
#ifdef DEBUG
#ifndef DEBUGGING_H
#define DEBUGGING_H
#include <stdio.h>

#define DBG_INFO(fmt, ...) fprintf(stderr, "[INFO] [%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);
#endif // DEBUGGING_H
#endif // DEBUG