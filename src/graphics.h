#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void draw_card(game_contextT *game_ctx);
void show_card_group(cartaT *group, const char *title, const char *title_fmt);
void show_player_state(game_contextT *game_ctx, giocatoreT *player);

#endif // GRAPHICS_H