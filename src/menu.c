#include <stdlib.h>
#include "menu.h"
#include "utils.h"
#include "files.h"
#include "game.h"
#include "saves.h"

game_contextT *main_menu(const char *provided_save) {
	int option;
	char *save_name;
	game_contextT *game_ctx;

	puts(MENU_ASCII_ART);

	if (provided_save == NULL) {
		do {
			puts("[" TO_STRING(MENU_NEWGAME) "] Avvia una nuova partita");
			puts("[" TO_STRING(MENU_LOADSAVE) "] Carica un salvataggio");
			puts("[" TO_STRING(MENU_QUIT) "] Esci dal gioco");
			option = get_int();
		} while (option < MENU_QUIT || option > MENU_LOADSAVE);

		switch (option) {
			case MENU_NEWGAME: {
				game_ctx = new_game();
				break;
			}
			case MENU_LOADSAVE: {
				save_name = pick_save();
				game_ctx = load_game(save_name);
				free_wrap(save_name);
				break;
			}
			case MENU_QUIT: {
				exit(EXIT_SUCCESS);
				break;
			}
		}
	} else
		game_ctx = load_game(provided_save);

	return game_ctx;
}