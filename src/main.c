// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "types.h"
#include "structs.h"
#include "utils.h"
#include "gameplay.h"
#include "card.h"
#include "graphics.h"

game_contextT* load_game() {
	// TODO: implement loading saved games
	return NULL;
}

void save_game(game_contextT* game_ctx) {
	// TODO: implement saving game
}


int main(int argc, char *argv[]) {
	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	assert(argc == 1);

	game_contextT* game_ctx = new_game();

	// game loop
	game_ctx->game_running = true;
	while (game_ctx->game_running) {
		save_game(game_ctx);

		begin_round(game_ctx);

		play_round(game_ctx);

		end_round(game_ctx);
	}



	clear_game(game_ctx);
	
}