#ifndef LOGGING_H
#define LOGGING_H

#include "types.h"
#include "structs.h"

void init_logging(game_contextT *game_ctx);
void shutdown_logging(game_contextT *game_ctx);
void log_msg(game_contextT *game_ctx, const char *msg);
void log_s(game_contextT *game_ctx, const char *fmt, const char *s0);
void log_ss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1);
void log_sss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1, const char *s2);

#endif // LOGGING_H