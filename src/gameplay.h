#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#include "types.h"


// actions
void view_own(game_contextT *game_ctx);
void view_others(game_contextT *game_ctx);
// end actions

void dispose_card(game_contextT *game_ctx, cartaT *card);

game_contextT *new_game();

void show_round(game_contextT *game_ctx);
void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando);
void apply_start_effects(game_contextT *game_ctx);
bool can_join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);
void join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);
void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);

void begin_round(game_contextT *game_ctx);
void play_round(game_contextT *game_ctx);
void end_round(game_contextT *game_ctx);

void clear_players(giocatoreT *head, giocatoreT *p);
void clear_game(game_contextT *game_ctx);

#endif // GAMEPLAY_H