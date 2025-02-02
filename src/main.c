// Nome: Diego Oliva (*REDACTED*)
// Matricola: *REDACTED*
// Tipologia progetto: avanzato

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "types.h"
#include "structs.h"
#include "gameplay.h"

int main(int argc, char *argv[]) {
	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	assert(argc == 1);

	game_contextT* game_ctx;
	// if (load_)
	// = new_game();

	// game loop
	game_ctx->game_running = true;
	while (game_ctx->game_running) {
		begin_round(game_ctx);

		play_round(game_ctx);

		end_round(game_ctx);
	}

	clear_game(game_ctx);
	
}