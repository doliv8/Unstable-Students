// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "types.h"
#include "structs.h"
#include "gameplay.h"
#include "logging.h"
#include "files.h"

int main(int argc, char *argv[]) {
	game_contextT* game_ctx;

	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	if (argc == 1)
		game_ctx = new_game();
	else
		game_ctx = load_game(argv[1]);

	init_logging(game_ctx);

	// game loop
	game_ctx->game_running = true;
	while (game_ctx->game_running) {
		begin_round(game_ctx);

		play_round(game_ctx);

		end_round(game_ctx);
	}

	clear_game(game_ctx);
	
}