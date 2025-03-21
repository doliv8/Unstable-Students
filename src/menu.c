#include <stdlib.h>
#include "menu.h"
#include "utils.h"
#include "files.h"
#include "game.h"
#include "saves.h"
#include "stats.h"

game_contextT *main_menu(const char *provided_save) {
	int option;
	char *save_name;
	game_contextT *game_ctx;
	bool in_menu = true;

	puts(MENU_ASCII_ART);

	if (provided_save == NULL) {
		while (in_menu) {
			do {
				puts("[" TO_STRING(MENU_NEWGAME) "] Avvia una nuova partita");
				puts("[" TO_STRING(MENU_LOADSAVE) "] Carica un salvataggio");
				puts("[" TO_STRING(MENU_STATS) "] Consulta le statistiche");
				puts("[" TO_STRING(MENU_QUIT) "] Esci dal gioco");
				option = get_int();
			} while (option < MENU_QUIT || option > MENU_STATS);

			switch (option) {
				case MENU_NEWGAME: {
					display_full_stats();
					game_ctx = new_game();
					in_menu = false;
					break;
				}
				case MENU_LOADSAVE: {
					save_name = pick_save();
					game_ctx = load_game(save_name);
					free_wrap(save_name);
					in_menu = false;
					break;
				}
				case MENU_STATS: {
					display_full_stats();
					break;
				}
				case MENU_QUIT: {
					exit(EXIT_SUCCESS);
					break;
				}
			}
		}
	} else
		game_ctx = load_game(provided_save);

	load_stats(game_ctx);

	return game_ctx;
}