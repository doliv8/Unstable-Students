#define _GNU_SOURCE

#include <string.h>
#include "game.h"
#include "card.h"
#include "files.h"
#include "logging.h"
#include "utils.h"

/**
 * @brief distributes cards at the start of the game to each player as described by the game rules
 * 
 * @param game_ctx 
 */
void distribute_cards(game_contextT *game_ctx) {
	cartaT *card;

	// distribute a MATRICOLA card for each player, rotating player for each given card
	for (int i = 0; i < game_ctx->n_players; i++, game_ctx->curr_player = game_ctx->curr_player->next) {
		card = pop_card(&game_ctx->aula_studio);
		push_card(&game_ctx->curr_player->aula, card);
	}

	// distribute a card CARDS_PER_PLAYER times the players count, rotating player for each given card
	for (int i = 0; i < CARDS_PER_PLAYER * game_ctx->n_players; i++, game_ctx->curr_player = game_ctx->curr_player->next) {
		card = pop_card(&game_ctx->mazzo_pesca);
		push_card(&game_ctx->curr_player->carte, card);
	}
}

/**
 * @brief create a new player (prompting user for the name)
 * 
 * @return giocatoreT* newly created player
 */
giocatoreT* new_player() {
	giocatoreT* player = (giocatoreT*)calloc_checked(1, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf(" %" TO_STRING(GIOCATORE_NAME_LEN) "[^\n]", player->name);
	} while (strnlen(player->name, sizeof(player->name)) == 0);

	return player;
}

/**
 * @brief create a new game context adding players, loading mazzo, initializing different decks and distributing cards
 * 
 * @return game_contextT* newly created game context
 */
game_contextT *new_game() {
	game_contextT *game_ctx = (game_contextT*)calloc_checked(1, sizeof(game_contextT));

	do {
		puts("Quanti giocatori giocheranno?");
		game_ctx->n_players = get_int();
	} while (game_ctx->n_players < MIN_PLAYERS || game_ctx->n_players > MAX_PLAYERS);

	// create players
	// game_ctx->curr_player serves as the linked-list head
	giocatoreT* curr_player;
	for (int i = 0; i < game_ctx->n_players; i++) {
		if (game_ctx->curr_player == NULL)
			curr_player = game_ctx->curr_player = new_player();
		else
			curr_player = curr_player->next = new_player();
	}
	curr_player->next = game_ctx->curr_player; // make the linked list circular linking tail to head

	// load cards
	int n_cards;
	cartaT *mazzo = load_mazzo(&n_cards);
	mazzo = shuffle_cards(mazzo, n_cards);

	game_ctx->aula_studio = split_matricole(&mazzo);
	game_ctx->mazzo_pesca = mazzo;

	distribute_cards(game_ctx);

	game_ctx->round_num = 1; // rounds start from 1

	return game_ctx;
}

/**
 * @brief recursive function to clear a giocatoreT* circular linked list
 * 
 * @param head pointer to the head of the circular linked list
 * @param p pointer to the node of the circular linked list to clear
 */
void clear_players(giocatoreT *head, giocatoreT *p) {
	// check for base case to recurse or not
	if (p->next != head)
		clear_players(head, p->next);

	// clear actual player
	if (p->aula != NULL)
		clear_cards(p->aula);
	if (p->bonus_malus != NULL)
		clear_cards(p->bonus_malus);
	if (p->carte != NULL)
		clear_cards(p->carte);
	free_wrap(p);
}

/**
 * @brief perform cleanup of the entire game, freeing players and every deck of cards
 * 
 * @param game_ctx 
 */
void clear_game(game_contextT *game_ctx) {
	clear_players(game_ctx->curr_player, game_ctx->curr_player);

	if (game_ctx->aula_studio != NULL)
		clear_cards(game_ctx->aula_studio);
	if (game_ctx->mazzo_pesca != NULL)
		clear_cards(game_ctx->mazzo_pesca);
	if (game_ctx->mazzo_scarti != NULL)
		clear_cards(game_ctx->mazzo_scarti);

	shutdown_logging(game_ctx);
	free_wrap(game_ctx);
}