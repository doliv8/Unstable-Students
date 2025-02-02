#define _GNU_SOURCE

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "gameplay.h"
#include "graphics.h"
#include "structs.h"
#include "card.h"
#include "files.h"
#include "utils.h"

bool has_bonusmalus(giocatoreT *player, azioneT effect_action) {
	bool found = false;
	for (cartaT *card = player->bonus_malus; card != NULL && !found; card = card->next) {
		for (int i = 0; i < card->n_effetti && !found; i++) {
			if (card->effetti[i].azione == effect_action)
				found = true;
		}
	}
	return found;
}

void show_player_state(game_contextT *game_ctx, giocatoreT *player) {
	printf("Ecco lo stato di " ANSI_UNDERLINE "%s" ANSI_RESET ":\n", player->name);

	printf("Numero carte nella mano: %d\n\n", count_cards(player->carte));
	if (has_bonusmalus(player, MOSTRA))
		show_card_group(player->carte, "Mano:", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET); // show mano
	show_card_group(player->aula, "Aula:", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
	show_card_group(player->bonus_malus, "Bonus/Malus:", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus
}

void view_own(game_contextT *game_ctx) {
	printf("Ecco le carte in tuo possesso, " ANSI_UNDERLINE "%s" ANSI_RESET ":\n\n", game_ctx->curr_player->name);

	show_card_group(game_ctx->curr_player->aula, "Aula:", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
	show_card_group(game_ctx->curr_player->bonus_malus, "Bonus/Malus:", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus
	show_card_group(game_ctx->curr_player->carte, "Mano:", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET); // show mano
}

void view_others(game_contextT *game_ctx) {
	giocatoreT *player;
	int chosen_idx;
	do {
		puts("Scegli il giocatore del quale vuoi vedere lo stato:");
		player = game_ctx->curr_player->next; // start from next player based on turns
		for (int i = 1; i < game_ctx->n_players; i++, player = player->next)
			printf(" [TASTO %d] %s\n", i, player->name);
		printf(" [TASTO %d] Tutti i giocatori\n", game_ctx->n_players);
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > game_ctx->n_players);

	player = game_ctx->curr_player->next; // start from next player based on turns
	for (int i = 1; i < game_ctx->n_players; i++, player = player->next) {
		if (i == chosen_idx || chosen_idx == game_ctx->n_players)
			show_player_state(game_ctx, player);
	}
}

// returns a linked list containing all the Matricola-kind cards found & removed from mazzo
cartaT *split_matricole(cartaT **mazzo_head) {
	cartaT *matricole_head = NULL;
	for (cartaT **prev = mazzo_head, *curr; *prev != NULL; ) {
		curr = *prev;
		if (curr->tipo == MATRICOLA) {
			// remove curr card from the mazzo linked list linking
			pop_card(prev);
			// link this matricola card to matricole_head linked_list
			push_card(&matricole_head, curr);
		} else
			prev = &curr->next;
	}
	return matricole_head;
}

cartaT *pick_card(cartaT *head, const char* prompt, const char *title, const char* title_fmt) {
	int n_cards = count_cards(head), chosen_idx;
	cartaT *card;
	show_card_group(head, title, title_fmt);
	do {
		puts(prompt);
		card = head;
		for (int idx = 1; idx <= n_cards; idx++, card = card->next)
			printf(" [TASTO %d] %s\n", idx, card->name);
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > n_cards);
	return card_by_index(head, chosen_idx);
}

void discard_card(game_contextT* game_ctx) {
	puts("Puoi avere massimo " ANSI_BOLD TO_STRING(ENDROUND_MAX_CARDS) ANSI_RESET " carte alla fine del round!");
	cartaT *card = pick_card(game_ctx->curr_player->carte, "Scegli la carta che vuoi scartare.",
		"Carte attualmente nella tua mano", ANSI_BOLD ANSI_RED "%s" ANSI_RESET
	);
	unlink_card(&game_ctx->curr_player->carte, card);
	printf("Hai scartato: %s\n", card->name);
	push_card(&game_ctx->mazzo_scarti, card); // put discarded card into mazzo scarti
}

int choice_action_menu() {
	int action;
	do {
		puts("Che azione vuoi eseguire?");
		puts(" [TASTO " TO_STRING(ACTION_PLAY_HAND) "] Gioca una carta dalla tua mano");
		puts(" [TASTO " TO_STRING(ACTION_DRAW) "] Pesca un'altra carta");
		puts(" [TASTO " TO_STRING(ACTION_VIEW_OWN) "] Visualizza le tue carte");
		puts(" [TASTO " TO_STRING(ACTION_VIEW_OTHERS) "] Visualizza lo stato degli altri giocatori");
		puts(" [TASTO " TO_STRING(ACTION_QUIT) "] Esci dalla partita");
		action = get_int();
	} while (action < ACTION_QUIT || action > ACTION_VIEW_OTHERS);
	return action;
}

void distribute_cards(game_contextT *game_ctx) {
	cartaT *card;

	// distribute a matricola card for each player, rotating player for each given card
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

giocatoreT* new_player() {
	giocatoreT* player = (giocatoreT*)calloc_checked(1, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf(" %" TO_STRING(GIOCATORE_NAME_LEN) "[^\n]", player->name);
	} while (!strnlen(player->name, sizeof(player->name)));

	return player;
}

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

	game_ctx->round_num = 1;

	return game_ctx;
}

void show_round(game_contextT *game_ctx) {
	printf("Round numero: " ANSI_BOLD "%d" ANSI_RESET "\n", game_ctx->round_num);

	printf("Ora gioca: " ANSI_UNDERLINE "%s" ANSI_RESET "\n", game_ctx->curr_player->name);


}

void apply_effects(game_contextT* game_ctx, cartaT* card) {

	if (card->opzionale) {
		puts("Vuoi applicare gli effetti di questa carta?");
		show_card(card);
		ask_choice();
		// TODO: implement choice
	}

	for (int i = 0; i < card->n_effetti; i++) {
		printf("*to implement* APPLYING an effect!\n");
		// TODO: apply actual effects
		// apply_effect(game_ctx, card->effetti[i]);
	}
}

void apply_start_effects(game_contextT *game_ctx) {
	giocatoreT *player = game_ctx->curr_player;
	// apply bonus malus quando = INIZIO effects
	for (cartaT *card = player->bonus_malus; card != NULL; card = card->next) {
		if (card->quando == INIZIO)
			apply_effects(game_ctx, card);
	}

	// apply aula quando = INIZIO effects
	for (cartaT *card = player->aula; card != NULL; card = card->next) {
		if (card->quando == INIZIO)
			apply_effects(game_ctx, card);
	}
}

void draw_card(game_contextT *game_ctx) {
	// shuffle and swap mazzo_scarti with mazzo_pesca if mazzo_pesca is empty
	if (game_ctx->mazzo_pesca == NULL) {
		game_ctx->mazzo_pesca = shuffle_cards(game_ctx->mazzo_scarti, count_cards(game_ctx->mazzo_scarti));
		game_ctx->mazzo_pesca = game_ctx->mazzo_scarti;
		game_ctx->mazzo_scarti = NULL; // mazzo_scarti has been moved to mazzo_pesca (emptied)
	}

	cartaT *drawn_card = pop_card(&game_ctx->mazzo_pesca);
	puts("Ecco la carta che hai pescato:");
	show_card(drawn_card);
	push_card(&game_ctx->curr_player->carte, drawn_card);
}

void begin_round(game_contextT *game_ctx) {
	save_game(game_ctx);

	show_round(game_ctx);
	apply_start_effects(game_ctx);

	draw_card(game_ctx);

}

void play_round(game_contextT *game_ctx) {
	bool in_action = true;
	while (in_action) {
		switch (choice_action_menu()) {
			case ACTION_PLAY_HAND: {
				// TODO: implement playing cards
				in_action = false;
				break;
			}
			case ACTION_DRAW: {
				draw_card(game_ctx);
				in_action = false;
				break;
			}
			case ACTION_VIEW_OWN: {
				view_own(game_ctx);
				break;
			}
			case ACTION_VIEW_OTHERS: {
				view_others(game_ctx);
				break;
			}
			case ACTION_QUIT: {
				printf("Are you sure you want to quit this game? ");
				if (ask_choice()) {
					game_ctx->game_running = false;
					in_action = false;
				}
				break;
			}
		}
	}
}

void end_round(game_contextT *game_ctx) {
	if (!game_ctx->game_running)
		return;

	// hand max cards check
	while (count_cards(game_ctx->curr_player->carte) > ENDROUND_MAX_CARDS)
		discard_card(game_ctx);

	// TODO: check win condition

	game_ctx->curr_player = game_ctx->curr_player->next; // next round its next player's turn
	game_ctx->round_num++;
}

// recursive function to clear a giocatoreT* circular linked list
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

void clear_game(game_contextT *game_ctx) {
	clear_players(game_ctx->curr_player, game_ctx->curr_player);

	if (game_ctx->aula_studio != NULL)
		clear_cards(game_ctx->aula_studio);
	if (game_ctx->mazzo_pesca != NULL)
		clear_cards(game_ctx->mazzo_pesca);
	if (game_ctx->mazzo_scarti != NULL)
		clear_cards(game_ctx->mazzo_scarti);
	free_wrap(game_ctx);
}
