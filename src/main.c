// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#include <stdlib.h>
#include <time.h>
#include "types.h"
#include "structs.h"
#include "gameplay.h"
#include "menu.h"
#include "game.h"

int main(int argc, const char *argv[]) {
	game_contextT* game_ctx;

	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	if (argc == ONE_ELEMENT) // no additional arguments are passed
		game_ctx = main_menu(NULL);
	else
		game_ctx = main_menu(argv[1]); // save path is passed as first command-line argument

	// game loop
	game_ctx->game_running = true;
	while (game_ctx->game_running) {
		begin_round(game_ctx);

		play_round(game_ctx);

		end_round(game_ctx);
	}

	clear_game(game_ctx);
	
}