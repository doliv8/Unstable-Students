// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#define _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include "types.h"
#include "structs.h"
#include "utils.h"

void duplicate_carta(cartaT* card) {
	cartaT* copy_card = (cartaT*)malloc_checked(sizeof(cartaT));
	*copy_card = *card; // copy the whole struct
	if (card->n_effetti != 0) {
		// create new effects array
		effettoT* effects = (effettoT*)malloc_checked(card->n_effetti*sizeof(effettoT));
		// copy effects into the new array
		for (int i = 0; i < card->n_effetti; i++)
			effects[i] = card->effetti[i];
		copy_card->effetti = effects;
	}
	card->next = copy_card;
}

effettoT* read_effetti(FILE* fp, int* n_effects) {
	int amount = 0;
	fscanf(fp, "%d", &amount);
	effettoT* effects = NULL;
	if (amount != 0)
		effects = (effettoT*)malloc_checked(amount*sizeof(effettoT));
	int val; // to hold numbers that get converted into enums
	for (int i = 0; i < amount; i++) {
		fscanf(fp, "%d", &val);
		effects[i].azione = val;
		fscanf(fp, "%d", &val);
		effects[i].target_giocatori = val;
		fscanf(fp, "%d", &val);
		effects[i].target_carta = val;
	}
	*n_effects = amount;
	return effects;
}

// returns number of cards of this type present, returns 0 if no more cards are readable from fp
int read_carta(FILE* fp, cartaT** ptr) {
	int amount;
	if (!fscanf(fp, "%d", &amount))
		return 0;

	cartaT base_card;
	int val; // to hold int values to later convert to enums

	fscanf(fp, "%" TO_STRING(CARTA_NAME_LEN) "[^\n]s", base_card.name);
	fscanf(fp, "%" TO_STRING(CARTA_DESCRIPTION_LEN) "[^\n]s", base_card.description);

	fscanf(fp, "%d", &val);
	base_card.tipo = (tipo_cartaT)val;
	
	base_card.effetti = read_effetti(fp, &base_card.n_effetti);

	fscanf(fp, "%d", &val);
	base_card.quando = (quandoT)val;
	fscanf(fp, "%d", &val);
	base_card.opzionale = (bool)val;

	for (int i = 0; i < amount; i++) {

	}

	// card->next = NULL;

	// *ptr = card;
	return amount;
}

void load_mazzo() {
	FILE* fp = fopen(FILE_MAZZO, "r");
	if (fp == NULL) {
		fprintf(stderr, "Opening cards file (%s) failed!\n", FILE_MAZZO);
		exit(EXIT_FAILURE);
	}

	cartaT* mazzo = NULL, **curr_ptr = &mazzo;
	int amount;
	do {
		amount = read_carta(fp, curr_ptr);
		if (amount > 1) {
			for (int i = 1; i < amount; i++) {
				duplicate_carta(*curr_ptr);
				curr_ptr = &(*curr_ptr)->next;
			}
		}
	} while (amount != 0);



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
	load_mazzo();


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