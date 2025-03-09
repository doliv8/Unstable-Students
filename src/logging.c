#include <stdio.h>
#include "logging.h"
#include "files.h"
#include "format.h"
#include "utils.h"

void init_logging(game_contextT *game_ctx) {
	game_ctx->log_file = open_log_append();
	fputs("Avvio del logging...\n", game_ctx->log_file);
}

void shutdown_logging(game_contextT *game_ctx) {
	fputs("Arresto del logging...\n", game_ctx->log_file);
	fclose(game_ctx->log_file);
}

void log_msg(game_contextT *game_ctx, const char *msg) {
	fprintf(game_ctx->log_file, "[Turno %d] %s\n", game_ctx->round_num, msg);
}

void log_s(game_contextT *game_ctx, const char *fmt, const char *s0) {
	char *msg;
	asprintf_s(&msg, fmt, s0);
	log_msg(game_ctx, msg);
	free_wrap(msg);
}

void log_ss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1) {
	char *msg;
	asprintf_ss(&msg, fmt, s0, s1);
	log_msg(game_ctx, msg);
	free_wrap(msg);
}

void log_sss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1, const char *s2) {
	char *msg;
	asprintf_sss(&msg, fmt, s0, s1, s2);
	log_msg(game_ctx, msg);
	free_wrap(msg);
}

void log_ssss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1, const char *s2, const char *s3) {
	char *msg;
	asprintf_ssss(&msg, fmt, s0, s1, s2, s3);
	log_msg(game_ctx, msg);
	free_wrap(msg);
}