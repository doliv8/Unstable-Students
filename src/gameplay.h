#ifndef GAMEPLAY_H
#define GAMEPLAY_H
#include "types.h"


// actions
void view_own(game_contextT *game_ctx);
void view_others(game_contextT *game_ctx);
// end actions

game_contextT *new_game();

void show_round(game_contextT *game_ctx);
void apply_effects(game_contextT *game_ctx, cartaT *card);
void apply_start_effects(game_contextT *game_ctx);

void begin_round(game_contextT *game_ctx);
void play_round(game_contextT *game_ctx);
void end_round(game_contextT *game_ctx);

void clear_players(giocatoreT *head, giocatoreT *p);
void clear_game(game_contextT *game_ctx);

#endif // GAMEPLAY_H