#ifndef FILES_H
#define FILES_H

#include "types.h"
#include "structs.h"

game_contextT* load_game();
void save_game(game_contextT* game_ctx);
cartaT *load_mazzo(int *n_cards);
FILE *open_log_write();

int read_int(FILE *fp);
int read_bin_int(FILE *fp);
void write_bin_int(FILE *fp, int val);

#endif // FILES_H