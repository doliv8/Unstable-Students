#ifndef FILES_H
#define FILES_H

#include "types.h"

game_contextT* load_game();
void save_game(game_contextT* game_ctx);
cartaT *load_mazzo(int *n_cards);

#endif // FILES_H