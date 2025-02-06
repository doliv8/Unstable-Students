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
			printf(" [TASTO %d] %s\n", i, player->name);
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


cartaT *pick_card_restricted(cartaT *head, tipo_cartaT type, const char* prompt, const char *title, const char* title_fmt) {
	int n_cards = count_cards_restricted(head, type), chosen_idx;
	cartaT *card;

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
			if (match_card_type(head, type))
				printf(" [TASTO %d] %s\n", idx++, card->name);
		}
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > n_cards);
	return card_by_index_restricted(head, type, chosen_idx);
}

cartaT *pick_card(cartaT *head, const char* prompt, const char *title, const char* title_fmt) {
	return pick_card_restricted(head, ALL, prompt, title, title_fmt);
}

// this function is similar to pick_card but specific to picking a card from a player's aula (bonus_malus + aula).
// first asks user if wants to pick from aula or bonus_malus and then calls actual pick_card on it.
cartaT *pick_aula_card(giocatoreT *player, const char *prompt) {
	int chosen_idx;
	cartaT *card;

	do {
		puts("Vuoi scegliere una carta dall'aula o dai bonus/malus?\n");
		puts(" [TASTO " TO_STRING(CHOICE_AULA) "] Aula");
		puts(" [TASTO " TO_STRING(CHOICE_BONUSMALUS) "] Bonus/Malus");
		chosen_idx = get_int();
	} while (chosen_idx < CHOICE_AULA || chosen_idx > CHOICE_BONUSMALUS);

	if (chosen_idx == CHOICE_AULA)
		card = pick_card(player->aula, prompt, "Aula", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
	else
		card = pick_card(player->bonus_malus, prompt, "Bonus/Malus", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
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

void play_card(game_contextT *game_ctx) {
	// TODO: implement this function

	cartaT *card = pick_card(game_ctx->curr_player->carte, "Scegli la carta che vuoi giocare.",
		"La tua mano", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
	);
	unlink_card(&game_ctx->curr_player->carte, card);
	printf("Hai scelto di giocare: %s\n", card->name);

	switch (card->tipo) {
		case ISTANTANEA:
		case MALUS:
		case MATRICOLA:
		case STUDENTE_SEMPLICE:
		case LAUREANDO: {
			// TODO: implement playing these cards
			puts("*non implementato*");
			break;
		}
		case BONUS: {
			if (has_bonusmalus(game_ctx->curr_player, IMPEDIRE)) {
				printf("Fin quando avrai l'effetto %s attivo, non puoui usare carte %s!",
					azioneT_str(IMPEDIRE),
					tipo_cartaT_str(BONUS)
				);
			} else
				assert(false); // TODO: handle this
			break;
		}
		case MAGIA: {
			// always quando = SUBITO, no additional checks needed
			apply_effects(game_ctx, card, SUBITO);
			dispose_card(game_ctx, card);
		}
		case ALL:
		case STUDENTE: {
			// not possible
			break;
		}
	}

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
	printf("Round numero: " ANSI_BOLD "%d" ANSI_RESET ".\n", game_ctx->round_num);

	printf("Ora gioca: " ANSI_UNDERLINE "%s" ANSI_RESET "!\n", game_ctx->curr_player->name);


}

void apply_effect_scambia(game_contextT *game_ctx) {
	giocatoreT *target = pick_player(game_ctx, "Scegli il giocatore col quale scambiare la tua mano:", false, false);

	printf("Hai scelto di scambiare il tuo mazzo con quello di " ANSI_UNDERLINE "%s" ANSI_RESET "!\n", target->name);

	cartaT *tmp = target->carte;
	target->carte = game_ctx->curr_player->carte;
	game_ctx->curr_player->carte = tmp;
}

void apply_effect_scarta(game_contextT *game_ctx, effettoT *effect) {
	// allowed values: target player = IO | VOI | TUTTI and target card = ALL
	giocatoreT *thrower = game_ctx->curr_player;
	if (effect->target_giocatori == IO)
		discard_card(game_ctx, &game_ctx->curr_player->carte, "Carte attualmente nella tua mano");
	else {
		// iterate through all target players using game_ctx->curr_player and make them discard one card
		if (effect->target_giocatori == VOI)
			game_ctx->curr_player = game_ctx->curr_player->next; // start from next if thrower is not included
		do {
			// TODO: check for MAI usage
			printf(
				ANSI_UNDERLINE "%s" ANSI_RESET " ti costringe a scartare una carta dalla tua mano, " ANSI_UNDERLINE "%s" ANSI_RESET "!\n",
				thrower->name, game_ctx->curr_player->name
			);
			discard_card(game_ctx, &game_ctx->curr_player->carte, "Carte attualmente nella tua mano");
			game_ctx->curr_player = game_ctx->curr_player->next;
		} while (game_ctx->curr_player != thrower);
	}
}

void apply_effect_elimina_io(game_contextT *game_ctx, effettoT *effect) {
	// allowed values: target player = IO and target card = ALL | STUDENTE | BONUS | MALUS
	cartaT *deleted;

	if (effect->target_carta == ALL)
		deleted = pick_aula_card(game_ctx->curr_player, "Scegli la carta che vuoi eliminare dalla tua aula.");
	else if (effect->target_carta == STUDENTE)
		deleted = pick_card_restricted(game_ctx->curr_player->aula, STUDENTE,
			"Scegli la carta Studente che vuoi eliminare dalla tua aula.",
			"Aula", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
		);
	else
		deleted = pick_card_restricted(game_ctx->curr_player->bonus_malus, effect->target_carta,
			"Scegli la carta che vuoi eliminare dalle tue Bonus/Malus.",
			"Aula", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
		);

	if (deleted != NULL) { // check if a card could be selected
		leave_aula(game_ctx, game_ctx->curr_player, deleted);
		dispose_card(game_ctx, deleted);
	}
}

void apply_effect_elimina_tu(game_contextT *game_ctx, effettoT *effect) {

}

void apply_effect_elimina_tutti(game_contextT *game_ctx, effettoT *effect) {

}

void apply_effect_elimina(game_contextT *game_ctx, effettoT *effect) {
	// allowed values: target player = IO | TU | TUTTI and target card = ALL | STUDENTE | BONUS | MALUS
	assert(effect->target_carta != ALL); // for now this case is not handled
	switch (effect->target_giocatori) {
		case IO:
			apply_effect_elimina_io(game_ctx, effect);
			break;
		case TU:
			apply_effect_elimina_tu(game_ctx, effect);
			break;
		case TUTTI:
			apply_effect_elimina_tutti(game_ctx, effect);
			break;
	}
}

void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card) {
	if (match_card_type(card, STUDENTE))
		unlink_card(&player->aula, card); // is STUDENTE
	else
		unlink_card(&player->bonus_malus, card); // is BONUS/MALUS
	// apply leave effects
	apply_effects(game_ctx, card, FINE);
}

void join_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card) {
	if (match_card_type(card, STUDENTE))
		push_card(&player->aula, card); // is STUDENTE
	else
		push_card(&player->bonus_malus, card); // is BONUS/MALUS
	// apply join effects
	apply_effects(game_ctx, card, INIZIO);
}

void apply_effect_ruba(game_contextT *game_ctx, effettoT *effect) {
	// allowed values: target player = TU and target card = STUDENTE | BONUS
	char *pick_player_prompt, *pick_card_prompt, *pick_card_title;
	giocatoreT *target;
	cartaT **target_cards, **own_cards, *card;

	asprintf_checked(&pick_player_prompt, "Scegli il giocatore al quale vuoi rubare una carta %s:",
		tipo_cartaT_str(effect->target_carta)
	);
	target = pick_player(game_ctx, pick_player_prompt, false, false);

	asprintf_checked(&pick_card_prompt, "Scegli la carta %s che vuoi rubare a " ANSI_UNDERLINE "%s" ANSI_RESET ":",
		tipo_cartaT_str(effect->target_carta), target->name
	);
	asprintf_checked(&pick_card_title, "Carte %s di %s", tipo_cartaT_str(effect->target_carta), target->name);

	if (effect->target_carta == BONUS) {
		target_cards = &target->bonus_malus;
		own_cards = &game_ctx->curr_player->bonus_malus;
	}
	else { // target is STUDENTE
		target_cards = &target->aula;
		own_cards = &game_ctx->curr_player->aula;
	}

	card = pick_card_restricted(*target_cards, effect->target_carta,
		pick_card_prompt, pick_card_title, ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
	);

	unlink_card(target_cards, card); // remove the selected card from the target cards of the target player
	printf("Hai rubato: %s\n", card->name);
	push_card(own_cards, card); // put stolen card into own cards
	// TODO: handle join into aula of STUDENTE

	free_wrap(pick_card_title);
	free_wrap(pick_card_prompt);
	free_wrap(pick_player_prompt);
}

void apply_effect(game_contextT *game_ctx, effettoT *effect) {
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
			apply_effect_elimina(game_ctx, effect);
			break;
		}
		case RUBA: {
			// allowed values: target player = TU and target card = STUDENTE | BONUS
			apply_effect_ruba(game_ctx, effect);
			break;
		}
		case PESCA: {
			// allowed values: target player = IO and target card = ALL
			draw_card(game_ctx);
			break;
		}
		case PRENDI: {
			// allowed values: target player = TU and target card = ALL
			break;
		}
		case SCAMBIA: {
			// allowed values: target player = TU and target card = ALL
			apply_effect_scambia(game_ctx);
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
	if (card->opzionale) {
		puts("Vuoi applicare gli effetti di questa carta?");
		show_card(card);
		apply = ask_choice();
	}

	if (apply) {
		// maybe consider "Effetti delle Carte con Azioni Multiple" from http://unstablegameswiki.com/index.php?title=Unstable_Unicorns_-_Second_Edition_Rules_-_Italian
		for (int i = 0; i < card->n_effetti; i++)
			apply_effect(game_ctx, &card->effetti[i]);
	}
}

void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando) {
	if (card->quando == quando)
		apply_effects_now(game_ctx, card);
}


void apply_start_effects(game_contextT *game_ctx) {
	giocatoreT *player = game_ctx->curr_player;

	// apply bonus malus quando = INIZIO effects
	for (cartaT *card = player->bonus_malus; card != NULL; card = card->next)
		apply_effects(game_ctx, card, INIZIO);
	// apply aula quando = INIZIO effects
	for (cartaT *card = player->aula; card != NULL; card = card->next)
		apply_effects(game_ctx, card, INIZIO);
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
				play_card(game_ctx);
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
	if (has_bonusmalus(game_ctx->curr_player, INGEGNERE))
		return false; // cant win with ingegnerizzazione
	return count_cards_restricted(game_ctx->curr_player->aula, STUDENTE) >= WIN_STUDENTS_COUNT;
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
