#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#include "types.h"


// actions
void view_own(game_contextT *game_ctx);
void view_others(game_contextT *game_ctx);
// end actions

giocatoreT *pick_player(game_contextT *game_ctx, const char *prompt, bool allow_self, bool allow_all);
cartaT *pick_card_restricted(cartaT *head, tipo_cartaT type, const char *prompt, const char *title, const char *title_fmt);

void dispose_card(game_contextT *game_ctx, cartaT *card);

game_contextT *new_game();

void show_round(game_contextT *game_ctx);
void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando);
void apply_start_effects(game_contextT *game_ctx);
bool can_join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);
void join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);
void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card, bool dispatch_effects);

void begin_round(game_contextT *game_ctx);
void play_round(game_contextT *game_ctx);
void end_round(game_contextT *game_ctx);

#endif // GAMEPLAY_H