#include <stdio.h>
#include "logging.h"
#include "files.h"
#include "format.h"
#include "utils.h"

/**
 * @brief write a message to logs
 * 
 * @param game_ctx 
 * @param msg message
 */
void log_msg(game_contextT *game_ctx, const char *msg) {
	fprintf(game_ctx->log_file, "%s\n", msg);
}

/**
 * @brief intializes logging for the given game context
 * 
 * @param game_ctx 
 */
void init_logging(game_contextT *game_ctx) {
	game_ctx->log_file = open_log_append();
	log_msg(game_ctx, "Avvio del logging...");
}

/**
 * @brief shuts down logging for the given game context
 * 
 * @param game_ctx 
 */
void shutdown_logging(game_contextT *game_ctx) {
	log_msg(game_ctx, "Arresto del logging...");
	fclose(game_ctx->log_file);
}

/**
 * @brief writes a round message to logs
 * 
 * @param game_ctx 
 * @param msg message
 */
void log_round(game_contextT *game_ctx, const char *msg) {
	fprintf(game_ctx->log_file, "[Turno %d] %s\n", game_ctx->round_num, msg);
}

/**
 * @brief writes a formatted round message to logs with one string parameter
 * 
 * @param game_ctx 
 * @param fmt format string
 * @param s0 param 1 (string)
 */
void log_s(game_contextT *game_ctx, const char *fmt, const char *s0) {
	char *msg;
	asprintf_s(&msg, fmt, s0);
	log_round(game_ctx, msg);
	free_wrap(msg);
}

/**
 * @brief writes a formatted round message to logs with two string parameters
 * 
 * @param game_ctx 
 * @param fmt format string
 * @param s0 param 1 (string)
 * @param s1 param 2 (string)
 */
void log_ss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1) {
	char *msg;
	asprintf_ss(&msg, fmt, s0, s1);
	log_round(game_ctx, msg);
	free_wrap(msg);
}

/**
 * @brief writes a formatted round message to logs with three string parameters
 * 
 * @param game_ctx 
 * @param fmt format string
 * @param s0 param 1 (string)
 * @param s1 param 2 (string)
 * @param s2 param 3 (string)
 */
void log_sss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1, const char *s2) {
	char *msg;
	asprintf_sss(&msg, fmt, s0, s1, s2);
	log_round(game_ctx, msg);
	free_wrap(msg);
}

/**
 * @brief writes a formatted round message to logs with four string parameters
 * 
 * @param game_ctx 
 * @param fmt format string
 * @param s0 param 1 (string)
 * @param s1 param 2 (string)
 * @param s2 param 3 (string)
 * @param s3 param 4 (string)
 */
void log_ssss(game_contextT *game_ctx, const char *fmt, const char *s0, const char *s1, const char *s2, const char *s3) {
	char *msg;
	asprintf_ssss(&msg, fmt, s0, s1, s2, s3);
	log_round(game_ctx, msg);
	free_wrap(msg);
}