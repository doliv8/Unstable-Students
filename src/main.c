// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#define _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include "types.h"
#include "structs.h"
#include "utils.h"

void load_mazzo() {
	FILE* fp = fopen(FILE_MAZZO, "r");
	if (fp == NULL) {
		
	}

	fclose(fp);
}

giocatoreT* new_player() {
	giocatoreT* player = (giocatoreT*)calloc_checked(1, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf("%" TO_STRING(GIOCATORE_NAME_LEN) "s", player->name);
	} while (!strnlen(player->name, sizeof(player->name)));

	return player;
}

game_contextT* new_game() {
	game_contextT* game_ctx = (game_contextT*)calloc_checked(1, sizeof(game_contextT));

	int n_giocatori;
	do {
		puts("Quanti giocatori giocheranno?");
		n_giocatori = get_int();
	} while (n_giocatori < MIN_PLAYERS || n_giocatori > MAX_PLAYERS);

	// game_ctx->next_player serves as the linked-list head
	giocatoreT* curr_player;
	for (int i = 0; i < n_giocatori; i++) {
		if (game_ctx->next_player == NULL)
			curr_player = game_ctx->next_player = new_player();
		else
			curr_player = curr_player->next = new_player();
	}
	curr_player->next = game_ctx->next_player; // make the linked list circular


	// ... TODO: load mazzo ...


	return game_ctx;
}

// recursive function to clear a giocatoreT* circular linked list
void clear_players(giocatoreT* head, giocatoreT* p) {
	// check for base case to recurse or not
	if (p->next != head)
		clear_players(head, p->next);

	// clear actual player
	// TODO: free carte
	free(p);
}

void clear_game(game_contextT* game_ctx) {

	clear_players(game_ctx->next_player, game_ctx->next_player);
	// TODO: free carte
	free(game_ctx);
}

int main(int argc, char *argv[]) {
	
	// check salvataggio
	assert(argc == 1);

	game_contextT* game_ctx = new_game();



	clear_game(game_ctx);
	
}