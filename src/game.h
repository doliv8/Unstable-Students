#ifndef GAME_H
#define GAME_H

#include "types.h"

void distribute_cards(game_contextT *game_ctx);
giocatoreT* new_player();
game_contextT *new_game();
void clear_players(giocatoreT *head, giocatoreT *p);
void clear_game(game_contextT *game_ctx);

#endif // GAME_H