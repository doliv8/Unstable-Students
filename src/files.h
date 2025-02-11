#ifndef FILES_H
#define FILES_H

#include "types.h"
#include "structs.h"

game_contextT* load_game();
void save_game(game_contextT* game_ctx);
cartaT *load_mazzo(int *n_cards);

int read_int(FILE *fp);
void write_int(FILE *fp, int val);

#endif // FILES_H