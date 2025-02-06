#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void show_card(cartaT *card);
void show_card_group(cartaT *group, const char *title, const char *title_fmt);
void show_card_group_restricted(cartaT *group, const char *title, const char *title_fmt, tipo_cartaT type);
void show_player_state(game_contextT *game_ctx, giocatoreT *player);
void show_round(game_contextT *game_ctx);

#endif // GRAPHICS_H