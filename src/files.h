#ifndef FILES_H
#define FILES_H

#include "types.h"
#include "structs.h"

game_contextT *load_game(const char *save_name);
void save_game(game_contextT *game_ctx);

cartaT *load_mazzo(int *n_cards);
FILE *open_log_append(void);
FILE *open_stats_read(void);
FILE *open_stats_read_write(void);
bool read_player_stats(FILE *fp, player_statsT *stats);
void write_player_stats(FILE *fp, player_statsT *stats);
void load_saves_cache(freeable_multiline_textT *saves);
void save_saves_cache(freeable_multiline_textT *saves);

#endif // FILES_H