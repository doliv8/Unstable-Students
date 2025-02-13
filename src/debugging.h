/*
* only used for debug builds (compiling with -DDEBUG parameter)
*/
#ifdef DEBUG
#ifndef DEBUGGING_H
#define DEBUGGING_H
#include <stdio.h>
#include "types.h"

#define DBG_INFO(fmt, ...) fprintf(stderr, "[INFO] [%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);

void dump_full_game(game_contextT *game_ctx);

#endif // DEBUGGING_H
#endif // DEBUG
