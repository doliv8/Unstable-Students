#define _GNU_SOURCE

#include <string.h>
#include "game.h"
#include "card.h"
#include "files.h"
#include "logging.h"
#include "utils.h"
#include "saves.h"

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
 * @param game_ctx 
 * @return giocatoreT* newly created player
 */
giocatoreT *new_player(game_contextT *game_ctx) {
	bool distinct;
	giocatoreT *player = (giocatoreT*)calloc_checked(ONE_ELEMENT, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf(" %" TO_STRING(GIOCATORE_NAME_LEN) "[^\n]", player->name);
		// check name differs from name of every other player inserted
		distinct = true;
		for (giocatoreT *other = game_ctx->curr_player; other != NULL && distinct; other = other->next) {
			if (!strncmp(player->name, other->name, sizeof(player->name)))
				distinct = false;
		}
	} while (!strnlen(player->name, sizeof(player->name)) || !distinct);

	return player;
}

/**
 * @brief create a new game context adding players, loading mazzo, initializing different decks and distributing cards
 * 
 * @return game_contextT* newly created game context
 */
game_contextT *new_game(void) {
	cartaT *mazzo;
	int n_cards;
	char *save_name;
	giocatoreT *curr_player = NULL;
	game_contextT *game_ctx = (game_contextT*)calloc_checked(ONE_ELEMENT, sizeof(game_contextT));

	init_logging(game_ctx);
	log_msg(game_ctx, "Creazione nuova partita...");

	save_name = ask_save_name(true);
	game_ctx->save_path = get_save_path(save_name);
	free_wrap(save_name);

	cache_save_name(game_ctx->save_path);

	do {
		puts("Quanti giocatori giocheranno?");
		game_ctx->n_players = get_int();
	} while (game_ctx->n_players < MIN_PLAYERS || game_ctx->n_players > MAX_PLAYERS);

	// create players
	// game_ctx->curr_player serves as the linked-list head
	for (int i = 0; i < game_ctx->n_players; i++) {
		if (game_ctx->curr_player == NULL && curr_player == NULL)
			curr_player = game_ctx->curr_player = new_player(game_ctx); // set linked list head
		else
			curr_player = curr_player->next = new_player(game_ctx);
	}
	curr_player->next = game_ctx->curr_player; // make the linked list circular linking tail to head

	// load cards
	mazzo = load_mazzo(&n_cards);
	fprintf(game_ctx->log_file, "Caricate %d carte nel mazzo!\n", n_cards);

	mazzo = shuffle_cards(mazzo);

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
 * @brief recursive function to clear a player_statsT* circular linked list
 * 
 * @param head pointer to the head of the circular linked list
 * @param p pointer to the node of the circular linked list to clear
 */
void clear_stats(player_statsT *head, player_statsT *stats) {
	// check for base case to recurse or not
	if (stats->next != head)
		clear_stats(head, stats->next);
	// free node 
	free_wrap(stats);
}

/**
 * @brief perform cleanup of the entire game, freeing players, stats and every deck of cards
 * 
 * @param game_ctx 
 */
void clear_game(game_contextT *game_ctx) {
	clear_players(game_ctx->curr_player, game_ctx->curr_player);
	clear_stats(game_ctx->curr_stats, game_ctx->curr_stats);

	if (game_ctx->aula_studio != NULL)
		clear_cards(game_ctx->aula_studio);
	if (game_ctx->mazzo_pesca != NULL)
		clear_cards(game_ctx->mazzo_pesca);
	if (game_ctx->mazzo_scarti != NULL)
		clear_cards(game_ctx->mazzo_scarti);

	log_msg(game_ctx, "Chiusura del gioco...");

	shutdown_logging(game_ctx);
	free_wrap(game_ctx->save_path);
	free_wrap(game_ctx);
}