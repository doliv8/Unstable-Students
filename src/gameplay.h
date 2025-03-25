#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#include "types.h"

bool is_self(game_contextT *game_ctx, giocatoreT *target);
void switch_player(game_contextT *game_ctx, giocatoreT *player);

bool target_defends(game_contextT *game_ctx, giocatoreT *target, cartaT *attack_card, effettoT *attack_effect);

// actions
bool play_card(game_contextT *game_ctx, tipo_cartaT type);
cartaT *draw_card(game_contextT *game_ctx);
void view_own(game_contextT *game_ctx);
void view_others(game_contextT *game_ctx);
// end actions

giocatoreT *pick_player(game_contextT *game_ctx, const char *prompt, bool allow_self, bool allow_all);
cartaT *pick_card(cartaT *head, tipo_cartaT type, const char *prompt, const char *title, const char *title_fmt);
cartaT *pick_random_card(cartaT *head, tipo_cartaT type);
cartaT *pick_aula_card(game_contextT *game_ctx, giocatoreT *target, tipo_cartaT type, const char *prompt);

void dispose_card(game_contextT *game_ctx, cartaT *card);
void discard_card(game_contextT *game_ctx, cartaT **cards, tipo_cartaT type, const char *title);

bool can_join_aula(giocatoreT *player, cartaT *card);
void join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card);
void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card, bool dispatch_effects);

void begin_round(game_contextT *game_ctx);
void play_round(game_contextT *game_ctx);
void end_round(game_contextT *game_ctx);

#endif // GAMEPLAY_H