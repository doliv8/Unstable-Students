#ifndef STATS_H
#define STATS_H

#include "types.h"

void display_full_stats();
void load_stats(game_contextT *game_ctx);
void save_stats(game_contextT *game_ctx);

void stats_add_win(game_contextT *game_ctx);
void stats_add_round(game_contextT *game_ctx);
void stats_add_discarded(game_contextT *game_ctx);
void stats_add_played_card(game_contextT *game_ctx, cartaT *card);

#endif // STATS_H