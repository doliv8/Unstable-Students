#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void show_card(cartaT *card);
void show_card_group(cartaT *group, const char *title, const char *title_fmt);
void show_player_state(game_contextT *game_ctx, giocatoreT *player);

#endif // GRAPHICS_H