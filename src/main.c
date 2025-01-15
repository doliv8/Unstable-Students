// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#define _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "structs.h"
#include "utils.h"

cartaT* duplicate_carta(cartaT* card) {
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
	return copy_card;
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

int read_int(FILE* fp) {
	int val;
	if (fscanf(fp, "%d", &val) != 1) {
		fprintf(stderr, "Error occurred while reading an integer from file stream!");
		exit(EXIT_FAILURE);
	}
	return val;
}

// returns the new linked list tail
// returns (through amount pointer) the number of cards of this type present, sets *amount to 0 if no more cards are readable from fp
// takes a pointer to the current tail's next pointer
cartaT* read_carta(FILE* fp, cartaT** tail_next, int* amount) {
	if (fscanf(fp, "%d", amount) != 1) {
		*amount = 0;
		return NULL;
	}
	
	cartaT* card = (cartaT*)malloc_checked(sizeof(cartaT));

	fscanf(fp, " %" TO_STRING(CARTA_NAME_LEN) "[^\n]", card->name);
	fscanf(fp, " %" TO_STRING(CARTA_DESCRIPTION_LEN) "[^\n]", card->description);

	card->tipo = (tipo_cartaT)read_int(fp);
	card->effetti = read_effetti(fp, &card->n_effetti);
	card->quando = (quandoT)read_int(fp);
	card->opzionale = read_int(fp) != 0;

	// always add first copy to the linked list
	*tail_next = card;
	// start from 1 as the first one has already been added to the linked list
	for (int i = 1; i < *amount; i++)
		card = card->next = duplicate_carta(card);

	// set last allocated card's next ptr to NULL and return the new linked list tail
	card->next = NULL;
	return card;
}

cartaT* load_mazzo(int* n_cards) {
	FILE* fp = fopen(FILE_MAZZO, "r");
	if (fp == NULL) {
		fprintf(stderr, "Opening cards file (%s) failed!\n", FILE_MAZZO);
		exit(EXIT_FAILURE);
	}

	*n_cards = 0;
	cartaT* mazzo = NULL, **tail_next = &mazzo, *new_tail;
	int amount;
	do {
		new_tail = read_carta(fp, tail_next, &amount);
		if (amount != 0) {
			tail_next = &new_tail->next;
			*n_cards += amount;
		}
	} while (amount != 0);

	fclose(fp);

	return mazzo;
}

// this function uses Fisher-Yates shuffle algorithm to shuffle the (linearized) dynamic array of cards in linear time
cartaT* shuffle_cards(cartaT* cards, int n_cards) {
	cartaT** linear_cards = malloc_checked(n_cards*sizeof(cartaT*));

	for (int i = 0; i < n_cards; i++) {
		// pop the card from the head of the linked list
		linear_cards[i] = cards;
		cards = cards->next;
	}

	// actual Fisher-Yates shuffling algorithm
	cartaT* temp; // to hold temporary cartaT pointer for swapping cards
	for (int i = n_cards-1, j; i > 0; i--) {
		j = rand_int(0, i);
		// swap cards at index i and j
		temp = linear_cards[i];
		linear_cards[i] = linear_cards[j];
		linear_cards[j] = temp;
	}

	// reconstruct the links between the nodes (cards) of the linked list following the shuffled order
	for (int i = 0; i < n_cards-1; i++)
		linear_cards[i]->next = linear_cards[i+1];
	linear_cards[n_cards-1]->next = NULL; // set tail next pointer to NULL

	cartaT* new_head =  linear_cards[0];

	// free the linear array of cards used during shuffle
	free(linear_cards);

	return new_head; // return new head to shuffled linked list
}

// returns a linked list containing all the Matricola-kind cards found & removed from mazzo
cartaT* split_matricole(cartaT** mazzo_head) {
	cartaT* matricole_head = NULL;
	for (cartaT** prev = mazzo_head, *curr; *prev != NULL; ) {
		curr = *prev;
		if (curr->tipo == MATRICOLA) {
			// remove curr card from the mazzo linked list linking
			*prev = curr->next;
			// link this matricola card to matricole_head linked_list
			curr->next = matricole_head;
			matricole_head = curr;
		} else
			prev = &curr->next;
	}

	return matricole_head;
}

giocatoreT* new_player() {
	giocatoreT* player = (giocatoreT*)calloc_checked(1, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf(" %" TO_STRING(GIOCATORE_NAME_LEN) "[^\n]", player->name);
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


	int n_cards;
	cartaT* mazzo = load_mazzo(&n_cards);
	mazzo = shuffle_cards(mazzo, n_cards);

	game_ctx->aula_studio = split_matricole(&mazzo);


	return game_ctx;
}

// recursive function to clear a giocatoreT* circular linked list
void clear_players(giocatoreT* head, giocatoreT* p) {
	// check for base case to recurse or not
	if (p->next != head)
		clear_players(head, p->next);

	// clear actual player
	// TODO: free carte
	free_wrap(p);
}

void clear_game(game_contextT* game_ctx) {

	clear_players(game_ctx->next_player, game_ctx->next_player);
	// TODO: free carte
	free_wrap(game_ctx);
}

int main(int argc, char *argv[]) {
	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	assert(argc == 1);

	game_contextT* game_ctx = new_game();



	clear_game(game_ctx);
	
}