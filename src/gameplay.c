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

#include <assert.h>

bool has_bonusmalus_target(giocatoreT *player, azioneT effect_action, cartaT *target) {
	bool found = false;
	for (cartaT *card = player->bonus_malus; card != NULL && !found; card = card->next) {
		for (int i = 0; i < card->n_effetti && !found; i++) {
			if (card->effetti[i].azione == effect_action && match_card_type(target, card->effetti[i].target_carta))
				found = true;
		}
	}
	return found;
}

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

// returns NULL when ALL is picked
giocatoreT *pick_player(game_contextT *game_ctx, const char *prompt, bool allow_self, bool allow_all) {
	giocatoreT *player;
	int chosen_idx, tot_players = allow_self ? game_ctx->n_players : game_ctx->n_players - 1,
		all_idx = tot_players + 1,
		max_choice = allow_all ? all_idx : tot_players;
	do {
		puts(prompt);
		player = allow_self ? game_ctx->curr_player : game_ctx->curr_player->next; // start from curr or next player based on turns
		for (int i = 1; i < all_idx; i++, player = player->next)
			printf(" [TASTO %d] %s%s\n", i, player->name, player == game_ctx->curr_player ? " (io)" : "");
		if (allow_all)
			printf(" [TASTO %d] Tutti i giocatori\n", all_idx);
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > max_choice);
	
	if (chosen_idx == all_idx)
		player = NULL; // signals ALL choice
	else {
		player = allow_self ? game_ctx->curr_player : game_ctx->curr_player->next;
		for (int i = 1; i < chosen_idx; i++)
			player = player->next;
	}
	return player;
}

void view_others(game_contextT *game_ctx) {
	giocatoreT *target = pick_player(game_ctx, "Scegli il giocatore del quale vuoi vedere lo stato:", false, true);
	if (target == NULL) { // picked option is ALL
		// start from next player based on turns
		for (giocatoreT *player = game_ctx->curr_player->next; player != game_ctx->curr_player; player = player->next)
			show_player_state(game_ctx, player);
	} else
		show_player_state(game_ctx, target);
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


/**
 * @brief prompts user to pick a card from the provided cards list with a restriction filter and returns the picked card
 * (still chained in the list).
 * 
 * @param head cards list
 * @param type card type user is allowed to pick
 * @param prompt text shown to the user while asked to pick the card
 * @param title title for the cards list box
 * @param title_fmt formatter for the title of the cards list box (visible length must be 0)
 * @return cartaT* picked card or NULL if there's no card to pick
 */
cartaT *pick_card_restricted(cartaT *head, tipo_cartaT type, const char* prompt, const char *title, const char* title_fmt) {
	cartaT *card;
	int n_cards = count_cards_restricted(head, type), chosen_idx;

	show_card_group_restricted(head, title, title_fmt, type);

	// handle no cards check
	if (n_cards == 0) {
		puts("Non ci sono carte da scegliere!");
		return NULL;
	}

	do {
		puts(prompt);
		card = head;
		for (int idx = 1; idx <= n_cards; card = card->next) {
			if (match_card_type(card, type))
				printf(" [TASTO %d] %s\n", idx++, card->name);
		}
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > n_cards);
	return card_by_index_restricted(head, type, chosen_idx);
}

/**
 * @brief prompts user to pick a card from the provided cards list and returns the picked card (still chained in the list).
 * 
 * @param head cards list
 * @param prompt text shown to the user while asked to pick the card
 * @param title title for the cards list box
 * @param title_fmt formatter for the title of the cards list box (visible length must be 0)
 * @return cartaT* picked card or NULL if there's no card to pick
 */
cartaT *pick_card(cartaT *head, const char* prompt, const char *title, const char* title_fmt) {
	return pick_card_restricted(head, ALL, prompt, title, title_fmt);
}

cartaT *pick_random_card_restricted(cartaT *head, tipo_cartaT type) {
	cartaT *card;
	int n_cards = count_cards_restricted(head, type), chosen_idx;

	if (n_cards != 0) {
		chosen_idx = rand_int(1, n_cards);
		card = card_by_index_restricted(head, type, chosen_idx);
	} else {
		puts("Non ci sono carte da estrarre!");
		card = NULL;
	}
	return card;
}

// this function is similar to pick_card but specific to picking a card from a player's aula (bonus_malus + aula).
// first asks user if wants to pick from aula or bonus_malus and then calls actual pick_card on it.
cartaT *pick_aula_card(giocatoreT *player, const char *prompt) {
	cartaT *card;
	int chosen_idx, n_aula = count_cards(player->aula), n_bonusmalus = count_cards(player->bonus_malus);

	if (n_aula + n_bonusmalus == 0) {
		printf("Non ci sono carte da scegliere nell'aula di %s!\n", player->name);
		return NULL;
	}

	if (n_bonusmalus == 0) { // only aula has cards
		card = pick_card(player->aula, prompt, "Aula", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
	} else if (n_aula == 0) { // only bonus/malus has cards
		card = pick_card(player->bonus_malus, prompt, "Bonus/Malus", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
	} else { // both aula and bonus/malus have cards
		do {
			// TODO: do dynamic title containing target player name (reusable in pick_card & show_card_grou).
			show_card_group(player->aula, "Aula:", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
			show_card_group(player->bonus_malus, "Bonus/Malus:", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus

			puts("Vuoi scegliere una carta dall'aula studenti o dai bonus/malus?\n");
			puts(" [TASTO " TO_STRING(CHOICE_AULA) "] Aula");
			puts(" [TASTO " TO_STRING(CHOICE_BONUSMALUS) "] Bonus/Malus");
			chosen_idx = get_int();
		} while (chosen_idx < CHOICE_AULA || chosen_idx > CHOICE_BONUSMALUS);

		if (chosen_idx == CHOICE_AULA)
			card = pick_card(player->aula, prompt, "Aula", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
		else // choice was bonus/malus
			card = pick_card(player->bonus_malus, prompt, "Bonus/Malus", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
	}
	return card;
}

void dispose_card(game_contextT *game_ctx, cartaT *card) {
	if (card->tipo == MATRICOLA)
		push_card(&game_ctx->aula_studio, card); // put MATRICOLA into aula studio
	else
		push_card(&game_ctx->mazzo_scarti, card); // put card into mazzo scarti
}

void discard_card(game_contextT* game_ctx, cartaT **cards, const char *title) {
	// handle no cards check
	if (count_cards(*cards) == 0) {
		puts("Avresti dovuto scartare una carta, ma non ne hai!");
		return;
	}
	cartaT *card = pick_card(*cards, "Scegli la carta che vuoi scartare.", title, ANSI_BOLD ANSI_RED "%s" ANSI_RESET);
	unlink_card(cards, card);
	printf("Hai scartato: %s\n", card->name);
	dispose_card(game_ctx, card); // dispose discarded card
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

int count_playable_cards(game_contextT *game_ctx) {
	int playable_cards = 0;
	for (cartaT *card = game_ctx->curr_player->carte; card != NULL; card = card->next) {
		if (card->tipo == MALUS || // Malus cards can be always played on self or other players
			(card->tipo != ISTANTANEA && // ISTANTANEA can't be played during own turn
			!has_bonusmalus_target(game_ctx->curr_player, IMPEDIRE, card) && // check for active IMPEDIRE effect
			!cards_contain(game_ctx->curr_player->aula, card) && // check for identical cards already in aula
			!cards_contain(game_ctx->curr_player->bonus_malus, card)) // check for identical Bonus cards already in bonus/malus
		)
			playable_cards++;
	}
	return playable_cards;
}

/**
 * @brief makes current player play a card from his hand
 * 
 * @param game_ctx 
 * @return true if player actually played a card
 * @return false player wasn't able to play a card
 */
bool play_card(game_contextT *game_ctx) {
	// TODO: implement this function
	bool played = false;
	cartaT *card;
	giocatoreT *target = game_ctx->curr_player;

	// handle no playable cards or no cards at all check
	if (count_playable_cards(game_ctx) == 0) {
		puts("Avresti dovuto giocare una carta ma non ne puoi giocare neanche una!");
		return false;
	}

	card = pick_card(game_ctx->curr_player->carte, "Scegli la carta che vuoi giocare.",
		"La tua mano", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
	);
	printf("Hai scelto di giocare: %s\n", card->name);

	switch (card->tipo) {
		case ISTANTANEA: {
			printf("Non puoi giocare una carta %s durante il tuo turno!\n", tipo_cartaT_str(ISTANTANEA));
			break;
		}
		case BONUS:
		case MALUS:
		case MATRICOLA:
		case STUDENTE_SEMPLICE:
		case LAUREANDO: {
			// check for active IMPEDIRE effect on this card type
			if (has_bonusmalus_target(game_ctx->curr_player, IMPEDIRE, card)) {
				printf("Fin quando avrai l'effetto %s attivo, non puoi usare carte %s!\n",
					azioneT_str(IMPEDIRE),
					tipo_cartaT_str(card->tipo)
				);
			} else {
				if (card->tipo == MALUS) // MALUS can be placed both in own and other player's bonusmalus
					target = pick_player(game_ctx, "Scegli un giocatore al quale applicare questa carta Malus.", true, false);
				if (can_join_aula(game_ctx, target, card)) {
					// TODO: check for MAI
					unlink_card(&game_ctx->curr_player->carte, card);
					join_aula(game_ctx, target, card);
					played = true;
				} else {
					if (target == game_ctx->curr_player)
						puts("Questa carta non puo' essere giocata dato che ne hai una uguale sul campo.");
					else
						printf("Questa carta non puo' essere giocata su %s dato che ne ha una uguale sul campo.\n", target->name);
				}
			}
			break;
		}
		case MAGIA: {
			// always quando = SUBITO, no additional checks needed
			unlink_card(&game_ctx->curr_player->carte, card);
			apply_effects(game_ctx, card, SUBITO);
			dispose_card(game_ctx, card);
			played = true;
		}
		case ALL:
		case STUDENTE: {
			// this code shouldn't be reachable
			break;
		}
	}
	return played ? true : play_card(game_ctx);
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

void apply_effect_scambia(game_contextT *game_ctx, giocatoreT **target_tu) {
	giocatoreT *target;

	if (*target_tu == NULL)
		*target_tu = pick_player(game_ctx, "Scegli il giocatore col quale scambiare la tua mano:", false, false);
	target = *target_tu;

	printf("Hai scelto di scambiare il tuo mazzo con quello di " ANSI_UNDERLINE "%s" ANSI_RESET "!\n", target->name);

	cartaT *tmp = target->carte;
	target->carte = game_ctx->curr_player->carte;
	game_ctx->curr_player->carte = tmp;
}

/**
 * @brief this effect makes players discard a card from their hands. for target VOI | TUTTI the card to discard is randomly picked.
 * 
 * @param game_ctx 
 * @param effect 
 */
void apply_effect_scarta(game_contextT *game_ctx, effettoT *effect) {
	// allowed values: target player = IO | VOI | TUTTI and target card = ALL
	giocatoreT *thrower = game_ctx->curr_player;
	cartaT *discarded_card;

	if (effect->target_giocatori == IO)
		discard_card(game_ctx, &game_ctx->curr_player->carte, "Carte attualmente nella tua mano");
	else {
		// iterate through all target players using game_ctx->curr_player and make them discard one card
		if (effect->target_giocatori == VOI)
			game_ctx->curr_player = game_ctx->curr_player->next; // start from next if thrower is not included
		do {
			// TODO: check for MAI usage
			printf(
				ANSI_UNDERLINE "%s" ANSI_RESET " ti fa scartare una carta dalla mano, " ANSI_UNDERLINE "%s" ANSI_RESET "!\n",
				thrower->name, game_ctx->curr_player->name
			);
			
			discarded_card = pick_random_card_restricted(game_ctx->curr_player->carte, effect->target_carta);
			if (discarded_card != NULL) {
				printf(ANSI_UNDERLINE "%s" ANSI_RESET " ha scartato %s!\n", game_ctx->curr_player->name, discarded_card->name);
				unlink_card(&game_ctx->curr_player->carte, discarded_card);
				dispose_card(game_ctx, discarded_card); // dispose discarded card
			}
			else
				printf(ANSI_UNDERLINE "%s" ANSI_RESET " non aveva carte da scartare nella sua mano!\n", game_ctx->curr_player->name);

			game_ctx->curr_player = game_ctx->curr_player->next;
		} while (game_ctx->curr_player != thrower);
	}
}

void apply_effect_elimina_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	char *prompt, *title;
	cartaT *deleted, *target_cards;

	if (effect->target_carta == ALL) {
		asprintf_s(&prompt, "Scegli la carta che vuoi eliminare dall'aula di %s.", target->name);
		deleted = pick_aula_card(target, prompt);
	} else { // handle STUDENTE and BONUS/MALUS
		asprintf_ss(&prompt, "Scegli la carta %s che vuoi eliminare dall'aula di %s.",
			tipo_cartaT_str(effect->target_carta), target->name
		);
		asprintf_s(&title, "Aula di %s", target->name);
		target_cards = effect->target_carta == STUDENTE ? target->aula : target->bonus_malus;
		deleted = pick_card_restricted(target_cards, effect->target_carta, prompt, title, ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET);
		free_wrap(title);
	}

	if (deleted != NULL) { // check if a card could be selected
		// TODO: check for MAI usage
		leave_aula(game_ctx, target, deleted);
		dispose_card(game_ctx, deleted);
	}

	free_wrap(prompt);
}

void apply_effect_elimina_tu(game_contextT *game_ctx, effettoT *effect, giocatoreT **target_tu) {
	char *pick_player_prompt;

	if (*target_tu == NULL) { // check if target tu was already asked in previous TU effects for this card
		asprintf_ss(&pick_player_prompt, "[%s]: Scegli il giocatore al quale vuoi eliminare una carta %s:",
			game_ctx->curr_player->name,
			tipo_cartaT_str(effect->target_carta)
		);
		*target_tu = pick_player(game_ctx, pick_player_prompt, false, false);
		free_wrap(pick_player_prompt);
	}

	apply_effect_elimina_target(game_ctx, *target_tu, effect);
}

void apply_effect_elimina_tutti(game_contextT *game_ctx, effettoT *effect) {
	giocatoreT *target;

	printf("Tutti i giocatori devono eliminare una carta %s!\n", tipo_cartaT_str(effect->target_carta));

	target = game_ctx->curr_player;
	for (int i = 0; i < game_ctx->n_players; i++, target = target->next)
		apply_effect_elimina_target(game_ctx, target, effect);
}

void apply_effect_elimina(game_contextT *game_ctx, effettoT *effect, giocatoreT **target_tu) {
	// allowed values: target player = IO | TU | TUTTI and target card = ALL | STUDENTE | BONUS | MALUS
	switch (effect->target_giocatori) {
		case IO:
			apply_effect_elimina_target(game_ctx, game_ctx->curr_player, effect);
			break;
		case TU:
			apply_effect_elimina_tu(game_ctx, effect, target_tu);
			break;
		case TUTTI:
			apply_effect_elimina_tutti(game_ctx, effect);
			break;
		default: // shouldn't be reachable
			break;
	}
}

/**
 * @brief applies leave effects of card and removes it from player's aula
 * 
 * @param game_ctx 
 * @param player target aula player
 * @param card leaving card
 */
void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card) {
	if (match_card_type(card, STUDENTE))
		unlink_card(&player->aula, card); // is STUDENTE
	else
		unlink_card(&player->bonus_malus, card); // is BONUS/MALUS
	// switch current player to card owner player for applying FINE effects correctly
	giocatoreT *original_player = game_ctx->curr_player;
	game_ctx->curr_player = player;
	// apply leave effects
	apply_effects(game_ctx, card, FINE);
	game_ctx->curr_player = original_player;
}

/**
 * @brief checks if card can join aula (no equal card is already in aula)
 * 
 * @param game_ctx 
 * @param player target aula player
 * @param card joining card
 * @return true if card can join aula
 * @return false if card couldn't join aula
 */
bool can_join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card) {
	bool can_join = true;
	if (match_card_type(card, STUDENTE)) { // is STUDENTE
		if (cards_contain(player->aula, card))
			can_join = false;
	} else { // is BONUS/MALUS
		if (cards_contain(player->bonus_malus, card))
			can_join = false;
	}
	return can_join;
}

/**
 * @brief pushes card to player's aula and applies its join effects. always call can_join_aula before calling this function.
 * 
 * @param game_ctx 
 * @param player target aula player
 * @param card joining card
 * @return true if card can join aula
 * @return false if card couldn't join aula
 */
void join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card) {
	if (match_card_type(card, STUDENTE)) // is STUDENTE
		push_card(&player->aula, card);
	else // is BONUS/MALUS
		push_card(&player->bonus_malus, card);
	apply_effects(game_ctx, card, SUBITO); // apply join effects
}

/**
 * @brief this effect lets you pick a target TU player and extracts a random card from the target player's hand;
 * that card gets removed from the target player's hand and put into the thrower player's hand.
 * 
 * @param game_ctx 
 * @param effect 
 * @param target_tu 
 */
void apply_effect_prendi(game_contextT *game_ctx, effettoT *effect, giocatoreT **target_tu) {
	// allowed values: target player = TU and target card = ALL
	char *pick_player_prompt;
	giocatoreT* target;
	cartaT *stolen_card;

	if (*target_tu == NULL) { // check if target tu was already asked in previous TU effects of the same card
		asprintf_s(&pick_player_prompt, "Scegli il giocatore al quale vuoi rubare una carta %s dal mazzo:",
			tipo_cartaT_str(effect->target_carta)
		);
		*target_tu = pick_player(game_ctx, pick_player_prompt, false, false);
	}
	target = *target_tu;

	// TODO: check for MAI usage

	stolen_card = pick_random_card_restricted(target->carte, effect->target_carta);
	if (stolen_card != NULL) {
		printf("Hai rubato %s da " ANSI_UNDERLINE "%s" ANSI_RESET "!\n", stolen_card->name, target->name);
		unlink_card(&target->carte, stolen_card); // remove extracted card from target's hand
		push_card(&game_ctx->curr_player->carte, stolen_card); // add extracted card to thrower's hand
	}
	else
		printf(ANSI_UNDERLINE "%s" ANSI_RESET " non aveva carte da rubare nella sua mano!\n", target->name);
}

void apply_effect_ruba(game_contextT *game_ctx, effettoT *effect, giocatoreT **target_tu) {
	// allowed values: target player = TU and target card = STUDENTE | BONUS
	char *pick_player_prompt, *pick_card_prompt, *pick_card_title;
	cartaT *target_cards, *card;
	giocatoreT* target;
	bool can_steal = false, stolen = false;

	if (*target_tu == NULL) { // check if target tu was already asked in previous TU effects of the same card
		asprintf_s(&pick_player_prompt, "Scegli il giocatore al quale vuoi rubare una carta %s:",
			tipo_cartaT_str(effect->target_carta)
		);
		*target_tu = pick_player(game_ctx, pick_player_prompt, false, false);
	}
	target = *target_tu;

	asprintf_ss(&pick_card_prompt, "Scegli la carta %s che vuoi rubare a " ANSI_UNDERLINE "%s" ANSI_RESET ":",
		tipo_cartaT_str(effect->target_carta), target->name
	);
	asprintf_ss(&pick_card_title, "Carte %s di %s", tipo_cartaT_str(effect->target_carta), target->name);

	target_cards = effect->target_carta == STUDENTE ? target->aula : target->bonus_malus;

	// check if any target card can be stolen by curr_player first
	for (card = target_cards; card != NULL && !can_steal; card = card->next) {
		if (match_card_type(card, effect->target_carta)) {
			if (can_join_aula(game_ctx, game_ctx->curr_player, card))
				can_steal = true;
		}
	}

	if (can_steal) {
		do {
			card = pick_card_restricted(target_cards, effect->target_carta,
				pick_card_prompt, pick_card_title, ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
			);

			// TODO: check for MAI
			if (can_join_aula(game_ctx, game_ctx->curr_player, card)) {
				leave_aula(game_ctx, target, card);
				join_aula(game_ctx, game_ctx->curr_player, card);
				printf("Hai rubato: %s\n", card->name);
				stolen = true;
			} else
				puts("Non puoi rubare questa carta dato che ne hai una uguale sul campo.");
		} while (!stolen);
	} else
		printf("%s non ha alcuna carta %s che puoi rubare!\n", target->name, tipo_cartaT_str(effect->target_carta));

	free_wrap(pick_card_title);
	free_wrap(pick_card_prompt);
	free_wrap(pick_player_prompt);
}

void apply_effect(game_contextT *game_ctx, effettoT *effect, giocatoreT **target_tu) {
	// TODO: apply ALL actual effects
	switch (effect->azione) {
		case GIOCA: {
			// allowed values: target player = IO and target card = ALL
			play_card(game_ctx);
			break;
		}
		case SCARTA: {
			// allowed values: target player = IO | VOI | TUTTI and target card = ALL
			apply_effect_scarta(game_ctx, effect);
			break;
		}
		case ELIMINA: {
			// allowed values: target player = IO | TU | TUTTI and target card = ALL | STUDENTE | BONUS | MALUS
			apply_effect_elimina(game_ctx, effect, target_tu);
			break;
		}
		case RUBA: {
			// allowed values: target player = TU and target card = STUDENTE | BONUS
			apply_effect_ruba(game_ctx, effect, target_tu);
			break;
		}
		case PESCA: {
			// allowed values: target player = IO and target card = ALL
			draw_card(game_ctx);
			break;
		}
		case PRENDI: {
			// allowed values: target player = TU and target card = ALL
			apply_effect_prendi(game_ctx, effect, target_tu);
			break;
		}
		case SCAMBIA: {
			// allowed values: target player = TU and target card = ALL
			apply_effect_scambia(game_ctx, target_tu);
			break;
		}
		case BLOCCA:
		case MOSTRA:
		case IMPEDIRE:
		case INGEGNERE: {
			// no action needs to be performed
			break;
		}
	}
}

void apply_effects_now(game_contextT *game_ctx, cartaT *card) {
	bool apply = true;
	giocatoreT *target_tu = NULL;

	if (card->opzionale) {
		puts("Vuoi applicare gli effetti di questa carta?");
		show_card(card);
		apply = ask_choice();
	}

	if (apply) {
		// maybe consider "Effetti delle Carte con Azioni Multiple" from http://unstablegameswiki.com/index.php?title=Unstable_Unicorns_-_Second_Edition_Rules_-_Italian
		for (int i = 0; i < card->n_effetti; i++)
			apply_effect(game_ctx, &card->effetti[i], &target_tu);
	}
}

void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando) {
	if (card->quando == quando)
		apply_effects_now(game_ctx, card);
}


void apply_start_effects(game_contextT *game_ctx) {
	giocatoreT *player = game_ctx->curr_player;
	cartaT **aula_cards;
	int n_cards = count_cards(player->bonus_malus) + count_cards(player->aula), idx;

	if (n_cards == 0) // no cards in aula, no need to apply any effect
		return;

	/* 
	 * dump bonus_malus and aula cards in a dynamic array to apply their effects in sequence without using their linking logic
	 * to avoid issues like: applying effects of removed cards or applying effects of another list if the current card gets moved
	 * into another cards linked list during the applying its effects (like Sparacoriandoli using ELIMINA on itself).
	*/

	idx = 0;
	aula_cards = (cartaT**)malloc_checked(n_cards*sizeof(cartaT*));

	// dump bonus/malus cards
	for (cartaT *card = player->bonus_malus; card != NULL; card = card->next, idx++)
		aula_cards[idx] = card;
	// dump aula cards
	for (cartaT *card = player->aula; card != NULL; card = card->next, idx++)
		aula_cards[idx] = card;

	for (idx = 0; idx < n_cards; idx++) {
		// first check if one of the aula or bonus_malus lists still contain the card or it got moved by any previous effect
		if (cards_contain_specific(player->bonus_malus, aula_cards[idx]) ||
			cards_contain_specific(player->aula, aula_cards[idx]))
				apply_effects(game_ctx, aula_cards[idx], INIZIO); // only apply effects of cards with quando = INIZIO
	}

	free_wrap(aula_cards);
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
				if (play_card(game_ctx)) // check for succesful play of card
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

bool check_win_condition(game_contextT *game_ctx) {
	bool can_win = has_bonusmalus(game_ctx->curr_player, INGEGNERE); // cant win with ingegnerizzazione
	return can_win && count_cards_restricted(game_ctx->curr_player->aula, STUDENTE) >= WIN_STUDENTS_COUNT; // TODO: dont count MATRICOLA here
}

void end_round(game_contextT *game_ctx) {
	if (!game_ctx->game_running)
		return;

	// hand max cards check
	while (count_cards(game_ctx->curr_player->carte) > ENDROUND_MAX_CARDS) {
		puts("Puoi avere massimo " ANSI_BOLD TO_STRING(ENDROUND_MAX_CARDS) ANSI_RESET " carte in mano alla fine del round!");
		discard_card(game_ctx, &game_ctx->curr_player->carte, "Carte attualmente nella tua mano");
	}

	if (check_win_condition(game_ctx)) { // check if curr player won
		printf("Congratulazioni " ANSI_UNDERLINE "%s" ANSI_RESET ", hai vinto la partita!\n", game_ctx->curr_player->name);
		game_ctx->game_running = false; // stop game
	} else { // no win, keep playing
		printf("Round di " ANSI_UNDERLINE "%s" ANSI_RESET " completato!\n", game_ctx->curr_player->name);
		game_ctx->curr_player = game_ctx->curr_player->next; // next round its next player's turn
		game_ctx->round_num++;
	}
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
