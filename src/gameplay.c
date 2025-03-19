#include <stdio.h>
#include "gameplay.h"
#include "graphics.h"
#include "format.h"
#include "structs.h"
#include "card.h"
#include "files.h"
#include "logging.h"
#include "utils.h"

/**
 * @brief checks if the provided target is current round's player
 * 
 * @param game_ctx 
 * @param target player to check
 * @return true if provided target is the current player
 * @return false if provided target isn't the current player
 */
bool is_self(game_contextT *game_ctx, giocatoreT *target) {
	return game_ctx->curr_player == target;
}

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

/**
 * @brief checks if a card can defend given target player from an attacking card.
 * 
 * @param target player the attack_card is being played on
 * @param defend_card candidate defense card (owned by target player)
 * @param attack_card attacking card to defend from
 * @return true if defend_card can block attack_card's effects
 * @return false if defend_card can't block attack_card's effects or can't even be played by target
 */
bool card_can_block(giocatoreT *target, cartaT *defend_card, cartaT *attack_card) {
	bool can_block = false;
	if (match_card_type(defend_card, ISTANTANEA) && // check if card type is ISTANTANEA
		defend_card->quando == SUBITO && // assert quando = SUBITO, just in case
		!has_bonusmalus_target(target, IMPEDIRE, defend_card)) { // check if card can be used (no IMPEDIRE is applied on it)
		for (int i = 0; i < defend_card->n_effetti && !can_block; i++) {
			if (defend_card->effetti[i].azione == BLOCCA && match_card_type(attack_card, defend_card->effetti[i].target_carta))
				can_block = true;
		}
	}
	return can_block;
}

/**
 * @brief checks if given target player can defend from an attacking card
 * 
 * @param target player the attack_card is being played on
 * @param attack_card attacking card to defend from
 * @return true if target player's hand has any card to defend against attack_card
 * @return false if target player can't defend from attack_card
 */
bool player_can_defend(giocatoreT *target, cartaT *attack_card) {
	bool can_defend = false;

	for (cartaT *card = target->carte; card != NULL && !can_defend; card = card->next) {
		if (card_can_block(target, card, attack_card))
			can_defend = true;
	}
	return can_defend;
}

/**
 * @brief allows target player to use ISTANTANEA card in reply to an effect inflicted by current player (mainly to use BLOCCA on it).
 * this function also applies effects of the defense card used.
 * 
 * @param game_ctx 
 * @param target player the attack_card is being played on
 * @param attack_card attacking card to defend from
 * @param attack_effect attacking effect to defend from
 * @return true if the attak was blocked by target
 * @return false if the attack wasn't blocked by target
 */
bool target_defends(game_contextT *game_ctx, giocatoreT *target, cartaT *attack_card, effettoT *attack_effect) {
	char *prompt, *effect_description, *attack_description, *attack_description_fmt;
	bool valid_defense = false, defends = false;
	cartaT *defense_card = NULL;
	giocatoreT *attacker = game_ctx->curr_player;

	if (attack_effect == CARD_PLACEMENT) {
		asprintf_ss(&attack_description, "dal piazzamento di '%s' nei %s", attack_card->name, tipo_cartaT_str(attack_card->tipo));
		asprintf_sss(&attack_description_fmt, "dal piazzamento di '%s' nei " COLORED_CARD_TYPE,
			attack_card->name,
			tipo_cartaT_color(attack_card->tipo),
			tipo_cartaT_str(attack_card->tipo)
		);
	}
	else {
		format_effect(&effect_description, attack_effect);
		asprintf_ss(&attack_description, "dall'attacco %s di '%s'", effect_description, attack_card->name);
		asprintf_ss(&attack_description_fmt, "dall'attacco " ANSI_BOLD "%s" ANSI_RESET " di '%s'", effect_description, attack_card->name);
	}

	if (player_can_defend(target, attack_card)) { // first check if target player can actually defend from the attack
		printf("[%s] Puoi difenderti %s da parte di " PRETTY_USERNAME ". Vuoi difenderti? ",
			target->name,
			attack_description_fmt,
			attacker->name
		);
		defends = ask_choice(); // then ask if target wants to defend from the attack
	}

	if (defends) { // user can and wants to defend from the attack
		asprintf_ssss(&prompt, "[%s] Scegli con quale carta " COLORED_CARD_TYPE " difenderti dall'attacco di " PRETTY_USERNAME ".",
			target->name,
			tipo_cartaT_color(ISTANTANEA),
			tipo_cartaT_str(ISTANTANEA),
			attacker->name
		);
		// ask target which defense card wants to use from his hand (only ISTANTANEA cards)
		do {
			defense_card = pick_card_restricted(target->carte, ISTANTANEA, prompt, "Istantanee nella tua mano", ANSI_BLUE "%s" ANSI_RESET);
			if (card_can_block(target, defense_card, attack_card)) // verify picked defense card can defend from the attack card
				valid_defense = true;
		} while (!valid_defense);
		free_wrap(prompt);

		printf(PRETTY_USERNAME " si difende %s da parte di " PRETTY_USERNAME " usando '%s'!\n",
			target->name, attack_description_fmt, attacker->name, defense_card->name
		);
		log_ssss(game_ctx, "%s si difende %s da parte di %s usando '%s'.",
			target->name, attack_description, attacker->name, defense_card->name
		);

		unlink_card(&target->carte, defense_card); // remove chosen defense card from target's hand
	
		game_ctx->curr_player = target; // switch current player to defending player for applying defense card effects correctly
		apply_effects(game_ctx, defense_card, SUBITO); // appply additional defense card effects
		game_ctx->curr_player = attacker; // switch back to attacking player

		dispose_card(game_ctx, defense_card); // dispose chosen defense card after its use ended
	}

	if (attack_effect != CARD_PLACEMENT)
		free_wrap(effect_description);
	free_wrap(attack_description);
	free_wrap(attack_description_fmt);

	return defends;
}

/**
 * @brief displays a player's public cards and number of private cards or even the private cards list if the player has an active MOSTRA effect
 * 
 * @param game_ctx 
 * @param player target player
 */
void show_player_state(game_contextT *game_ctx, giocatoreT *player) {
	printf("Ecco lo stato di " PRETTY_USERNAME ":\n", player->name);

	if (has_bonusmalus(player, MOSTRA))
		show_card_group(player->carte, "Mano:", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET); // show mano
	else
		printf("Numero carte nella mano: %d\n", count_cards(player->carte));
	show_card_group(player->aula, "Aula:", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
	show_card_group(player->bonus_malus, "Bonus/Malus:", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus
}

/**
 * @brief displays player's own cards (private ones included)
 * 
 * @param game_ctx 
 */
void view_own(game_contextT *game_ctx) {
	printf("Ecco le carte in tuo possesso, " PRETTY_USERNAME ":\n", game_ctx->curr_player->name);

	show_card_group(game_ctx->curr_player->aula, "Aula:", ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
	show_card_group(game_ctx->curr_player->bonus_malus, "Bonus/Malus:", ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus
	show_card_group(game_ctx->curr_player->carte, "Mano:", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET); // show mano
}

/**
 * @brief allows to pick a player or every player to then show their states through show_player_state
 * 
 * @param game_ctx 
 */
void view_others(game_contextT *game_ctx) {
	giocatoreT *target = pick_player(game_ctx, "Scegli il giocatore del quale vuoi vedere lo stato:", !ALLOW_SELF, ALLOW_ALL);
	if (target == NULL) { // picked option is ALL
		// start from next player based on turns
		for (giocatoreT *player = game_ctx->curr_player->next; player != game_ctx->curr_player; player = player->next)
			show_player_state(game_ctx, player);
	} else
		show_player_state(game_ctx, target);
}

/**
 * @brief prompts user to pick a player from the game
 * 
 * @param game_ctx 
 * @param prompt message displayed during the picking
 * @param allow_self should picking self be an option?
 * @param allow_all should all players be an option?
 * @return giocatoreT* pointer to selected player or NULL if all players is picked
 */
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

/**
 * @brief prompts user to pick a card from the provided cards list with a restriction filter and returns the picked card
 * (still chained in the list).
 * 
 * @param head cards list
 * @param type card type user is allowed to pick
 * @param prompt text shown to the user while asked to pick the card
 * @param title title for the cards list box
 * @param title_fmt formatter for the title of the cards list box (visible length must be 0)
 * @return cartaT* pointer to picked card or NULL if there's no card to pick
 */
cartaT *pick_card_restricted(cartaT *head, tipo_cartaT type, const char *prompt, const char *title, const char *title_fmt) {
	cartaT *card;
	int chosen_idx, n_cards = count_cards_restricted(head, type);

	show_card_group_restricted(head, title, title_fmt, type);

	// handle no cards check
	if (n_cards == 0) {
		printf("Non ci sono carte " COLORED_CARD_TYPE " da scegliere!\n",
			tipo_cartaT_color(type),
			tipo_cartaT_str(type)
		);
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
 * @return cartaT* pointer to picked card or NULL if there's no card to pick
 */
cartaT *pick_card(cartaT *head, const char* prompt, const char *title, const char* title_fmt) {
	return pick_card_restricted(head, ALL, prompt, title, title_fmt);
}

/**
 * @brief automatically picks a card from the provided cards list with a restriction filter and returns the picked card
 * (still chained in the list).
 * 
 * @param head cards list
 * @param type card type user is allowed to pick
 * @return cartaT* pointer to picked card or NULL if there's no card to pick
 */
cartaT *pick_random_card_restricted(cartaT *head, tipo_cartaT type) {
	cartaT *card;
	int n_cards = count_cards_restricted(head, type), chosen_idx;

	if (n_cards != 0) {
		chosen_idx = rand_int(1, n_cards); // pick a 1-indexed index of the restricted cards
		card = card_by_index_restricted(head, type, chosen_idx);
	} else {
		puts("Non ci sono carte da estrarre!");
		card = NULL;
	}
	return card;
}

/**
 * @brief this function is similar to pick_card but specific to picking a card from a player's aula (bonus_malus + aula).
 * if specified type allows both BONUS/MALUS and STUDENT (is ALL) and there are cards both in bonus_malus and aula, asks
 * user if wants to pick from aula or bonus_malus and then calls actual pick_card on the chosen list. otherwise, pick_card
 * is directly called on the only available chosable list (bonus_malsu or aula).
 * 
 * @param game_ctx 
 * @param target player to pick an aula card from
 * @param type allowed target type (ALL | STUDENTE | MATRICOLA | STUDENTE_SEMPLICE | LAUREANDO | BONUS | MALUS)
 * @param prompt shown when picking the card from one of the lists
 * @return cartaT* pointer to picked card or NULL if no cards were present
 */
cartaT *pick_aula_card(game_contextT *game_ctx, giocatoreT *target, tipo_cartaT type, const char *prompt) {
	cartaT *card;
	char *aula_title, *bonusmalus_title;
	int chosen_idx, n_aula = count_cards_restricted(target->aula, type), n_bonusmalus = count_cards_restricted(target->bonus_malus, type);

	if (n_aula + n_bonusmalus == 0) {
		if (is_self(game_ctx, target))
			printf("[%s] Non ci sono carte " COLORED_CARD_TYPE " da scegliere nella tua aula!\n",
				game_ctx->curr_player->name,
				tipo_cartaT_color(type),
				tipo_cartaT_str(type)
			);
		else
			printf("Non ci sono carte " COLORED_CARD_TYPE " da scegliere nell'aula di " PRETTY_USERNAME "!\n",
				tipo_cartaT_color(type),
				tipo_cartaT_str(type),
				target->name);
		return NULL;
	}

	// build dynamic titles containing target player name and target card type
	if (is_self(game_ctx, target)) {
		if (type == ALL) {
			aula_title = strdup_checked("La tua aula");
			bonusmalus_title = strdup_checked("Le tue Bonus/Malus");
		} else { // pickable cards can only be in aula or bonusmalus lists (not both)
			// only one of these titles will be used, make them equal. must contain target card type
			asprintf_s(&aula_title, "Le tue carte %s", tipo_cartaT_str(type));
			bonusmalus_title = strdup_checked(aula_title);
		}
	} else {
		if (type == ALL) {
			asprintf_s(&aula_title, "Aula di %s", target->name);
			asprintf_s(&bonusmalus_title, "Bonus/Malus di %s", target->name);
		} else { // pickable cards can only be in aula or bonusmalus lists (not both)
			// only one of these titles will be used, make them equal. must contain target player name and target card type
			asprintf_ss(&aula_title, "Carte %s di %s", tipo_cartaT_str(type), target->name);
			bonusmalus_title = strdup_checked(aula_title);
		}
	}

	if (n_bonusmalus == 0) { // only aula has cards
		card = pick_card_restricted(target->aula, type, prompt, aula_title, ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
	} else if (n_aula == 0) { // only bonus/malus has cards
		card = pick_card_restricted(target->bonus_malus, type, prompt, bonusmalus_title, ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
	} else { // both aula and bonus/malus have cards and given type must be ALL
		do {
			show_card_group(target->aula, aula_title, ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET); // show aula
			show_card_group(target->bonus_malus, bonusmalus_title, ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET); // show bonus/malus

			if (is_self(game_ctx, target))
				printf("[%s] Vuoi scegliere una carta " COLORED_CARD_TYPE " dalla tua aula studenti o dai tuoi Bonus/Malus?\n",
					game_ctx->curr_player->name,
					tipo_cartaT_color(type),
					tipo_cartaT_str(type)
				);
			else
				printf("[%s] Vuoi scegliere una carta " COLORED_CARD_TYPE " dall'aula studenti o dai Bonus/Malus di " PRETTY_USERNAME "?\n",
					game_ctx->curr_player->name,
					tipo_cartaT_color(type),
					tipo_cartaT_str(type),
					target->name
				);
			puts(" [TASTO " TO_STRING(CHOICE_AULA) "] Aula");
			puts(" [TASTO " TO_STRING(CHOICE_BONUSMALUS) "] Bonus/Malus");
			chosen_idx = get_int();
		} while (chosen_idx < CHOICE_AULA || chosen_idx > CHOICE_BONUSMALUS);

		if (chosen_idx == CHOICE_AULA)
			card = pick_card_restricted(target->aula, type, prompt, aula_title, ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
		else // choice was bonus/malus
			card = pick_card_restricted(target->bonus_malus, type, prompt, bonusmalus_title, ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
	}
	free_wrap(bonusmalus_title);
	free_wrap(aula_title);
	return card;
}

void dispose_card(game_contextT *game_ctx, cartaT *card) {
	if (match_card_type(card, MATRICOLA))
		push_card(&game_ctx->aula_studio, card); // put MATRICOLA into aula studio
	else
		push_card(&game_ctx->mazzo_scarti, card); // put card into mazzo scarti
}

void discard_card(game_contextT *game_ctx, cartaT **cards, tipo_cartaT type, const char *title) {
	cartaT *card = pick_card_restricted(*cards, type, "Scegli la carta che vuoi scartare.", title, ANSI_BOLD ANSI_RED "%s" ANSI_RESET);

	if (card != NULL) {
		unlink_card(cards, card);
		dispose_card(game_ctx, card); // dispose discarded card
		printf("Hai scartato: %s\n", card->name);
		log_ss(game_ctx, "%s ha scartato '%s'.", game_ctx->curr_player->name, card->name);
	}
	else {
		printf("Avresti dovuto scartare una carta " COLORED_CARD_TYPE ", ma non ne hai!\n", tipo_cartaT_color(type), tipo_cartaT_str(type));
		log_ss(game_ctx, "%s avrebbe dovuto scartare una carta %s, ma non ne aveva.", game_ctx->curr_player->name, tipo_cartaT_str(type));
	}
}

/**
 * @brief makes current player draw a card from the mazzo pesca and if empty swaps it with mazzo scarti.
 * 
 * @param game_ctx 
 * @return cartaT* the drawn card
 */
cartaT *draw_card(game_contextT *game_ctx) {
	cartaT *drawn_card;

	// shuffle and swap mazzo_scarti with mazzo_pesca if mazzo_pesca is empty
	if (game_ctx->mazzo_pesca == NULL) {
		game_ctx->mazzo_pesca = shuffle_cards(game_ctx->mazzo_scarti);
		game_ctx->mazzo_pesca = game_ctx->mazzo_scarti;
		game_ctx->mazzo_scarti = NULL; // mazzo_scarti has been moved to mazzo_pesca (emptied)
	}

	drawn_card = pop_card(&game_ctx->mazzo_pesca);
	puts("Ecco la carta che hai pescato:");
	show_card(drawn_card);
	log_ss(game_ctx, "%s ha pescato '%s'.", game_ctx->curr_player->name, drawn_card->name);
	push_card(&game_ctx->curr_player->carte, drawn_card);
	return drawn_card;
}

int count_playable_cards(game_contextT *game_ctx, tipo_cartaT type) {
	// TODO: re-read this and check it's working properly
	int playable_cards = 0;
	for (cartaT *card = game_ctx->curr_player->carte; card != NULL; card = card->next) {
		if (match_card_type(card, type) && // check for card matching card type
			card->tipo != ISTANTANEA && // ISTANTANEA can't be played during own turn
			!has_bonusmalus_target(game_ctx->curr_player, IMPEDIRE, card)) { // check for active IMPEDIRE effects on this card
			playable_cards++;
		}
	}
	return playable_cards;
}

/**
 * @brief makes current player play a card from his hand
 * 
 * @param game_ctx 
 * @param type card type allowed to play
 * @return true if player actually played a card
 * @return false player couldn't play a card
 */
bool play_card(game_contextT *game_ctx, tipo_cartaT type) {
	bool played = false;
	cartaT *card;
	giocatoreT *target, *thrower;
	char *playable_prompt, *player_prompt;

	target = thrower = game_ctx->curr_player;

	// handle no playable cards or no cards at all check
	if (count_playable_cards(game_ctx, type) == 0) {
		printf("Avresti dovuto giocare una carta " COLORED_CARD_TYPE " ma non ne puoi giocare neanche una!\n",
			tipo_cartaT_color(type),
			tipo_cartaT_str(type)
		);
		log_ss(game_ctx, "%s avrebbe dovuto giocare una carta %s ma non ne aveva di giocabili.", thrower->name, tipo_cartaT_str(type));
		return false;
	}

	if (type != ALL)
		asprintf_ss(&playable_prompt, "Scegli la carta " COLORED_CARD_TYPE " che vuoi giocare.",
			tipo_cartaT_color(type),
			tipo_cartaT_str(type)
		);
	else
		playable_prompt = strdup_checked("Scegli la carta che vuoi giocare.");

	card = pick_card_restricted(thrower->carte, type, playable_prompt,
		"La tua mano", ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET
	);
	printf("Hai scelto di giocare: %s\n", card->name);

	// check for active IMPEDIRE effect on this card type
	if (has_bonusmalus_target(thrower, IMPEDIRE, card)) {
		printf("Fin quando avrai l'effetto %s attivo, non puoi usare carte " COLORED_CARD_TYPE "!\n",
			azioneT_str(IMPEDIRE),
			tipo_cartaT_color(card->tipo),
			tipo_cartaT_str(card->tipo)
		);
		log_ssss(game_ctx, "%s ha provato a giocare '%s' ma ha l'effetto %s attivo su carte %s.",
			thrower->name,
			card->name,
			azioneT_str(IMPEDIRE),
			tipo_cartaT_str(card->tipo)
		);
	} else {
		switch (card->tipo) {
			case ISTANTANEA: {
				printf("Non puoi giocare una carta " COLORED_CARD_TYPE " durante il tuo turno!\n",
					tipo_cartaT_color(ISTANTANEA),
					tipo_cartaT_str(ISTANTANEA)
				);
				break;
			}
			case BONUS:
			case MALUS:
			case MATRICOLA:
			case STUDENTE_SEMPLICE:
			case LAUREANDO: {
				// BONUS and MALUS can be placed both in own and other player's bonusmalus
				if (match_card_type(card, BONUS) || match_card_type(card, MALUS)) {
					asprintf_sss(&player_prompt, "Scegli un giocatore al quale piazzare '%s' nei " COLORED_CARD_TYPE ".",
						card->name,
						tipo_cartaT_color(card->tipo),
						tipo_cartaT_str(card->tipo)
					);
					target = pick_player(game_ctx, player_prompt, ALLOW_SELF, !ALLOW_ALL);
					free_wrap(player_prompt);
				}
				if (can_join_aula(game_ctx, target, card)) {
					log_sss(game_ctx, "%s gioca '%s' su %s.", thrower->name, card->name, target->name);
					unlink_card(&thrower->carte, card);
					if (target == thrower || !target_defends(game_ctx, target, card, CARD_PLACEMENT)) // can't defended from self thrown cards
						join_aula(game_ctx, target, card);
					else
						dispose_card(game_ctx, card);
					played = true;
				} else {
					if (target == thrower)
						printf("Questa carta (%s) non puo' essere piazzata nella tua aula dato che ne hai gia' una uguale.\n", card->name);
					else
						printf("Questa carta (%s) non puo' essere piazzata nell'aula di %s dato che ne ha gia' una uguale.\n",
							card->name,
							target->name
						);
					printf("Puoi comunque giocare questa carta ma verrebbe scartata, confermi? ");
					if (ask_choice()) { // user still wants to play the card
						unlink_card(&thrower->carte, card);
						dispose_card(game_ctx, card);
						puts("Carta scartata!");
						log_sss(game_ctx, "%s ha provato a giocare '%s' su %s (duplicato), scartandola.", thrower->name, card->name, target->name);
						played = true;
					}
				}
				break;
			}
			case MAGIA: {
				// always quando = SUBITO, no additional checks needed
				log_ss(game_ctx, "%s gioca '%s'.", thrower->name, card->name);
				unlink_card(&thrower->carte, card);
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
	}

	free_wrap(playable_prompt);

	return played ? true : play_card(game_ctx, type); // recurse if player didnt play anything (but actually could)
}

/**
 * @brief prompts user to pick an option from action menu
 * 
 * @return int picked action menu option
 */
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

/**
 * @brief applies SCARTA effect on the given target.
 * this effect makes targets discard a card from their hands.
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect SCARTA effect
 */
void apply_effect_scarta_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	char *title;
	cartaT *discarded_card;

	if (is_self(game_ctx, target)) { // target is self, picking which card to discard is allowed
		printf("[%s] Devi scartare una carta " COLORED_CARD_TYPE " dal tuo mazzo!\n",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);

		if (effect->target_carta == ALL)
			title = strdup_checked("La tua mano");
		else
			asprintf_s(&title, "%s nella tua mano", tipo_cartaT_str(effect->target_carta));

		discard_card(game_ctx, &game_ctx->curr_player->carte, effect->target_carta, title);
		free_wrap(title);
	} else { // target is another player, random card extraction is used
		printf("[%s] " PRETTY_USERNAME " ti fa scartare una carta " COLORED_CARD_TYPE " dalla mano!\n",
			target->name,
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		discarded_card = pick_random_card_restricted(target->carte, effect->target_carta);
		if (discarded_card != NULL) {
			unlink_card(&target->carte, discarded_card);
			dispose_card(game_ctx, discarded_card); // dispose discarded card
			printf(PRETTY_USERNAME " ha scartato '%s'!\n", target->name, discarded_card->name);
			log_sss(game_ctx, "%s ha scartato %s a causa dell'attacco di %s.",
				target->name,
				discarded_card->name,
				game_ctx->curr_player->name
			);
		} else {
			printf(PRETTY_USERNAME " non aveva carte " COLORED_CARD_TYPE " da scartare nella sua mano!\n",
				target->name,
				tipo_cartaT_color(effect->target_carta),
				tipo_cartaT_str(effect->target_carta)
			);
			log_sss(game_ctx, "%s doveva scartare una carta %s a causa dell'attacco di %s, ma non ne aveva.",
				target->name,
				tipo_cartaT_str(effect->target_carta),
				game_ctx->curr_player->name
			);
		}
	}
}

/**
 * @brief applies ELIMINA effect on the given target.
 * this effect makes thrower pick which card to delete from targets's aula (aula + bonusmalus).
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect ELIMINA effect
 */
void apply_effect_elimina_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	char *prompt;
	cartaT *deleted;

	if (is_self(game_ctx, target)) {
		printf("[%s] Devi eliminare una carta " COLORED_CARD_TYPE " dalla tua aula.\n",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		asprintf_sss(&prompt, "[%s] Scegli la carta " COLORED_CARD_TYPE " che vuoi eliminare dalla tua aula.",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		deleted = pick_aula_card(game_ctx, game_ctx->curr_player, effect->target_carta, prompt);
		if (deleted != NULL) {
			printf("[%s] Hai scelto di eliminare '%s' dalla tua aula!\n", game_ctx->curr_player->name, deleted->name);
			log_ss(game_ctx, "%s ha scelto di eliminare '%s' dalla sua aula.", game_ctx->curr_player->name, deleted->name);
		} else {
			log_ss(game_ctx, "%s avrebbe dovuto eliminare una carta %s dalla sua aula, ma non ne aveva.",
				game_ctx->curr_player->name,
				tipo_cartaT_str(effect->target_carta)
			);
		}
	} else {
		printf("[%s] Devi eliminare una carta " COLORED_CARD_TYPE " dall'aula di " PRETTY_USERNAME ".\n",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta),
			target->name
		);
		asprintf_ssss(&prompt, "[%s] Scegli la carta " COLORED_CARD_TYPE " che vuoi eliminare dall'aula di " PRETTY_USERNAME ".",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta),
			target->name
		);
		deleted = pick_aula_card(game_ctx, target, effect->target_carta, prompt);
		if (deleted != NULL) {
			printf(PRETTY_USERNAME " ha eliminato '%s' dall'aula di " PRETTY_USERNAME "!\n",
				game_ctx->curr_player->name,
				deleted->name,
				target->name
			);
			log_sss(game_ctx, "%s ha eliminato '%s' dall'aula di %s.", game_ctx->curr_player->name, deleted->name, target->name);
		} else {
			log_sss(game_ctx, "%s avrebbe dovuto eliminare una carta %s dall'aula di %s, ma non ne aveva.",
				game_ctx->curr_player->name,
				tipo_cartaT_str(effect->target_carta),
				target->name
			);
		}
	}

	if (deleted != NULL) { // check if a card could be selected
		leave_aula(game_ctx, target, deleted, DISPATCH_EFFECTS);
		dispose_card(game_ctx, deleted);
	}

	free_wrap(prompt);
}

/**
 * @brief applies GIOCA effect on the given target.
 * this effect makes target play a card.
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect GIOCA effect
 */
void apply_effect_gioca_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	giocatoreT *thrower = game_ctx->curr_player;

	if (is_self(game_ctx, target)) {
		printf("[%s] Devi giocare una carta " COLORED_CARD_TYPE " dal tuo mazzo.\n",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		log_sss(game_ctx, "%s deve giocare una carta %s grazie all'effetto %s.",
			game_ctx->curr_player->name,
			tipo_cartaT_str(effect->target_carta),
			azioneT_str(effect->azione)
		);
		if (!play_card(game_ctx, effect->target_carta)) {
			log_ss(game_ctx, "%s avrebbe dovuto giocare una carta %s, ma non ne aveva.",
				target->name,
				tipo_cartaT_str(effect->target_carta)
			);
		}
	} else {
		printf("[%s] " PRETTY_USERNAME " ti fa giocare una carta " COLORED_CARD_TYPE " dal tuo mazzo.\n",
			target->name,
			thrower->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		log_ssss(game_ctx, "%s deve giocare una carta %s grazie all'effetto %s di %s.",
			target->name,
			tipo_cartaT_str(effect->target_carta),
			azioneT_str(effect->azione),
			thrower->name
		);

		game_ctx->curr_player = target; // switch current player to the target player to create a sub-round for target to play a card
		if (!play_card(game_ctx, effect->target_carta)) {
			log_ss(game_ctx, "%s avrebbe dovuto giocare una carta %s, ma non ne aveva.",
				target->name,
				tipo_cartaT_str(effect->target_carta)
			);
		}
		game_ctx->curr_player = thrower; // switch back to original card thrower player
	}
}

/**
 * @brief applies PESCA effect on the given target.
 * this effect makes target draw a card.
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect PESCA effect
 */
void apply_effect_pesca_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	cartaT *drawn_card;
	giocatoreT *thrower = game_ctx->curr_player;

	if (is_self(game_ctx, target)) {
		printf("[%s] Devi pescare una carta " COLORED_CARD_TYPE ".\n",
			game_ctx->curr_player->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		log_sss(game_ctx, "%s deve pescare una carta %s grazie all'effetto %s.",
			game_ctx->curr_player->name,
			tipo_cartaT_str(effect->target_carta),
			azioneT_str(effect->azione)
		);
		drawn_card = draw_card(game_ctx);
		if (!match_card_type(drawn_card, effect->target_carta)) {
			// dispose card as it is not of the specified type
			unlink_card(&game_ctx->curr_player->carte, drawn_card); // remove drawn card from hand
			dispose_card(game_ctx, drawn_card);
			printf("[%s] Avresti dovuto pescare una carta " COLORED_CARD_TYPE " ma hai pescato '%s' (" COLORED_CARD_TYPE "), che viene quindi scartata!\n",
				game_ctx->curr_player->name,
				tipo_cartaT_color(effect->target_carta),
				tipo_cartaT_str(effect->target_carta),
				drawn_card->name,
				tipo_cartaT_color(drawn_card->tipo),
				tipo_cartaT_str(drawn_card->tipo)
			);
			log_ssss(game_ctx, "%s avrebbe dovuto pescare una carta %s, ma ha pescato '%s' (%s), scartandola.",
				target->name,
				tipo_cartaT_str(effect->target_carta),
				drawn_card->name,
				tipo_cartaT_str(drawn_card->tipo)
			);
		}
	} else {
		printf("[%s] " PRETTY_USERNAME " ti fa pescare una carta " COLORED_CARD_TYPE ".\n",
			target->name,
			thrower->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		log_ssss(game_ctx, "%s deve pescare una carta %s grazie all'effetto %s di %s.",
			target->name,
			tipo_cartaT_str(effect->target_carta),
			azioneT_str(effect->azione),
			thrower->name
		);
		game_ctx->curr_player = target; // switch current player to the target player to create a sub-round for target to draw a card
		drawn_card = draw_card(game_ctx);
		if (!match_card_type(drawn_card, effect->target_carta)) {
			// dispose card as it is not of the specified type
			unlink_card(&game_ctx->curr_player->carte, drawn_card); // remove drawn card from hand
			dispose_card(game_ctx, drawn_card);
			printf(PRETTY_USERNAME " avrebbe dovuto pescare una carta " COLORED_CARD_TYPE " ma ha pescato '%s' (" COLORED_CARD_TYPE "), che viene quindi scartata!\n",
				game_ctx->curr_player->name,
				tipo_cartaT_color(effect->target_carta),
				tipo_cartaT_str(effect->target_carta),
				drawn_card->name,
				tipo_cartaT_color(drawn_card->tipo),
				tipo_cartaT_str(drawn_card->tipo)
			);
			log_ssss(game_ctx, "%s avrebbe dovuto pescare una carta %s, ma ha pescato '%s' (%s), scartandola.",
				target->name,
				tipo_cartaT_str(effect->target_carta),
				drawn_card->name,
				tipo_cartaT_str(drawn_card->tipo)
			);
		}
		game_ctx->curr_player = thrower; // switch back to original card thrower player
	}
}

/**
 * @brief applies leave effects of card and removes it from player's aula
 * 
 * @param game_ctx 
 * @param player target aula player
 * @param card leaving card
 * @param dispatch_effects should apply card's FINE effects?
 */
void leave_aula(game_contextT *game_ctx, giocatoreT *player, cartaT *card, bool dispatch_effects) {
	giocatoreT *original_player;

	if (match_card_type(card, STUDENTE))
		unlink_card(&player->aula, card); // is STUDENTE
	else
		unlink_card(&player->bonus_malus, card); // is BONUS/MALUS

	log_sss(game_ctx, "Una carta %s lascia l'aula di %s: '%s'.", tipo_cartaT_str(card->tipo), player->name, card->name);
	if (dispatch_effects) {
		// switch current player to card owner player for applying FINE effects correctly
		original_player = game_ctx->curr_player;
		game_ctx->curr_player = player;
		// apply leave effects
		apply_effects(game_ctx, card, FINE);
		game_ctx->curr_player = original_player;
	}
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
	log_sss(game_ctx, "Una carta %s entra nell'aula di %s: '%s'.", tipo_cartaT_str(card->tipo), player->name, card->name);
	apply_effects(game_ctx, card, SUBITO); // apply join effects
}

/**
 * @brief applies PRENDI effect on the given target.
 * this effect extracts and removes a random card from the target player's hand putting it into the thrower player's hand.
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect PRENDI effect
 */
void apply_effect_prendi_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	cartaT *stolen_card;

	if (is_self(game_ctx, target)) { // cards like this shouldn't exist
		puts("Non puoi rubare una carta a te stesso!");
		return;
	}

	printf("[%s] " PRETTY_USERNAME " ti ruba una carta " COLORED_CARD_TYPE " dalla mano!\n",
		target->name,
		game_ctx->curr_player->name,
		tipo_cartaT_color(effect->target_carta),
		tipo_cartaT_str(effect->target_carta)
	);
	stolen_card = pick_random_card_restricted(target->carte, effect->target_carta);
	if (stolen_card != NULL) {
		unlink_card(&target->carte, stolen_card); // remove extracted card from target's hand
		push_card(&game_ctx->curr_player->carte, stolen_card); // add extracted card to thrower's hand
		printf("Hai rubato '%s' dalla mano di " PRETTY_USERNAME "!\n", stolen_card->name, target->name);
		log_sss(game_ctx, "%s ha rubato '%s' dalla mano di %s.", game_ctx->curr_player->name, stolen_card->name, target->name);
	} else {
		printf(PRETTY_USERNAME " non aveva carte " COLORED_CARD_TYPE " da rubare nella sua mano!\n",
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta),
			target->name
		);
		log_sss(game_ctx, "%s doveva rubare una carta %s dalla mano di %s, ma non ne aveva.",
			game_ctx->curr_player->name,
			tipo_cartaT_str(effect->target_carta),
			target->name
		);
	}
}

/**
 * @brief applies RUBA effect on the given target.
 * this effect makes thrower pick which card to steal from targets's aula (aula + bonusmalus).
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect RUBA effect
 */
void apply_effect_ruba_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	char *prompt, *title;
	bool can_steal = false, stolen = false;
	cartaT *card, *target_cards = effect->target_carta == STUDENTE ? target->aula : target->bonus_malus;

	if (is_self(game_ctx, target)) { // cards like this shouldn't exist
		puts("Non puoi rubare una carta a te stesso!");
		return;
	}

	printf("[%s] Devi rubare una carta " COLORED_CARD_TYPE " dall'aula di " PRETTY_USERNAME ".\n",
		game_ctx->curr_player->name,
		tipo_cartaT_color(effect->target_carta),
		tipo_cartaT_str(effect->target_carta),
		target->name
	);
	asprintf_ssss(&prompt, "[%s] Scegli la carta " COLORED_CARD_TYPE " che vuoi rubare a " PRETTY_USERNAME ".",
		game_ctx->curr_player->name,
		tipo_cartaT_color(effect->target_carta),
		tipo_cartaT_str(effect->target_carta),
		target->name
	);
	asprintf_ss(&title, "Carte %s di %s", tipo_cartaT_str(effect->target_carta), target->name);

	// check if any target card can be stolen by curr_player first
	for (card = target_cards; card != NULL && !can_steal; card = card->next) {
		if (match_card_type(card, effect->target_carta)) {
			if (can_join_aula(game_ctx, game_ctx->curr_player, card))
				can_steal = true;
		}
	}

	if (can_steal) {
		do {
			card = pick_card_restricted(target_cards, effect->target_carta, prompt, title, ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET);

			if (can_join_aula(game_ctx, game_ctx->curr_player, card)) {
				leave_aula(game_ctx, target, card, DISPATCH_EFFECTS);
				join_aula(game_ctx, game_ctx->curr_player, card);
				printf("Hai rubato: %s\n", card->name);
				log_sss(game_ctx, "%s ha rubato '%s' a %s.", game_ctx->curr_player->name, card->name, target->name);
				stolen = true;
			} else
				printf("Non puoi rubare '%s' dato che ne hai una uguale nella tua aula.\n", card->name);
		} while (!stolen);
	} else {
		printf(PRETTY_USERNAME " non ha alcuna carta " COLORED_CARD_TYPE " che puoi rubare!\n",
			target->name,
			tipo_cartaT_color(effect->target_carta),
			tipo_cartaT_str(effect->target_carta)
		);
		log_sss(game_ctx, "%s avrebbe dovuto rubare una carta %s a %s, ma non ne aveva.",
			game_ctx->curr_player->name,
			tipo_cartaT_str(effect->target_carta),
			target->name
		);
	}

	free_wrap(title);
	free_wrap(prompt);
}

/**
 * @brief applies SCAMBIA effect on the given target.
 * this effect swaps thrower and target player's hands.
 * 
 * @param game_ctx 
 * @param target player to activate the effect on
 * @param effect SCAMBIA effect
 */
void apply_effect_scambia_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect) {
	cartaT *thrower_cards = game_ctx->curr_player->carte;

	if (is_self(game_ctx, target)) {
		puts("Hai scambiato la mano con te stesso!");
		// no need to actually do anything :)
		return;
	}

	printf(PRETTY_USERNAME " ha scambiato la sua mano con quella di " PRETTY_USERNAME "!\n", game_ctx->curr_player->name, target->name);
	log_ss(game_ctx, "%s scambia il suo mazzo con quello di %s.", game_ctx->curr_player->name, target->name);

	// swap hands
	game_ctx->curr_player->carte = target->carte;
	target->carte = thrower_cards;
}

/**
 * @brief this function just calls the given effect with the specified target
 * 
 * @param game_ctx 
 * @param effect 
 * @param target 
 */
void apply_effect_target(game_contextT *game_ctx, effettoT *effect, giocatoreT *target) {
	switch (effect->azione) {
		case GIOCA: {
			// allowed values: target player = * and target card = *
			apply_effect_gioca_target(game_ctx, target, effect);
			break;
		}
		case SCARTA: {
			// allowed values: target player = * and target card = *
			apply_effect_scarta_target(game_ctx, target, effect);
			break;
		}
		case ELIMINA: {
			// allowed values: target player = * and target card = *
			apply_effect_elimina_target(game_ctx, target, effect);
			break;
		}
		case RUBA: {
			// allowed values: target player = * and target card = *
			apply_effect_ruba_target(game_ctx, target, effect);
			break;
		}
		case PESCA: {
			// allowed values: target player = * and target card = *
			apply_effect_pesca_target(game_ctx, target, effect);
			break;
		}
		case PRENDI: {
			// allowed values: target player = * and target card = *
			apply_effect_prendi_target(game_ctx, target, effect);
			break;
		}
		case SCAMBIA: {
			// allowed values: target player = * and target card = ALL
			apply_effect_scambia_target(game_ctx, target, effect);
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

/**
 * @brief 
 * 
 * @param game_ctx 
 * @param card attacking card
 * @param effect effect to apply
 * @param target_tu pointer to target_tu variable, to use for target player = TU
 * @return true if effect was blocked
 * @return false if effect wasn't blocked
 */
bool apply_effect(game_contextT *game_ctx, cartaT *card, effettoT *effect, giocatoreT **target_tu) {
	char *pick_player_prompt;
	giocatoreT *target;
	bool blocked = false;

	switch (effect->target_giocatori) {
		case IO: {
			// can't defend from self thrown cards so just apply the effect
			apply_effect_target(game_ctx, effect, game_ctx->curr_player);
			break;
		}
		case TU: {
			if (*target_tu == NULL) { // check if target tu was already asked in previous TU effects for this card
				// two different cases for messages (SCAMBIA didn't respect same phrase composition as other actions)
				if (effect->azione == SCAMBIA)
					asprintf_s(&pick_player_prompt, "[%s]: Scegli il giocatore col quale vuoi scambiare il tuo mazzo.",
						game_ctx->curr_player->name
					);
				else
					asprintf_ssss(&pick_player_prompt, "[%s]: Scegli il giocatore al quale vuoi %s una carta " COLORED_CARD_TYPE ".",
						game_ctx->curr_player->name,
						azioneT_verb_str(effect->azione),
						tipo_cartaT_color(effect->target_carta),
						tipo_cartaT_str(effect->target_carta)
					);
				*target_tu = pick_player(game_ctx, pick_player_prompt, !ALLOW_SELF, !ALLOW_ALL);
				free_wrap(pick_player_prompt);
			}
			if (!target_defends(game_ctx, *target_tu, card, effect))
				apply_effect_target(game_ctx, effect, *target_tu);
			else
				blocked = true;
			break;
		}
		case VOI:
		case TUTTI: {
			// there are 4 cases: VOI + SCAMBIA action, TUTTI + SCAMBIA action, VOI + generic action, TUTTI + generic action
			if (effect->target_giocatori == VOI) {
				if (effect->azione == SCAMBIA)
					printf(PRETTY_USERNAME " deve scambiare il suo mazzo con tutti i giocatori (eccetto " PRETTY_USERNAME ")!\n",
						game_ctx->curr_player->name,
						game_ctx->curr_player->name
					);
				else
					printf(PRETTY_USERNAME " deve %s una carta " COLORED_CARD_TYPE " a tutti i giocatori (eccetto " PRETTY_USERNAME ")!\n",
						game_ctx->curr_player->name,
						azioneT_verb_str(effect->azione),
						tipo_cartaT_color(effect->target_carta),
						tipo_cartaT_str(effect->target_carta),
						game_ctx->curr_player->name
					);
				target = game_ctx->curr_player->next; // start from next player if curr player is not included
			} else { // TUTTI
				if (effect->azione == SCAMBIA)
					printf(PRETTY_USERNAME " deve scambiare il suo mazzo con tutti i giocatori!\n", game_ctx->curr_player->name);
				else
					printf(PRETTY_USERNAME " deve %s una carta " COLORED_CARD_TYPE " a tutti i giocatori!\n",
						game_ctx->curr_player->name,
						"eliminare dall'aula",
						tipo_cartaT_color(effect->target_carta),
						tipo_cartaT_str(effect->target_carta)
					);
				target = game_ctx->curr_player; // start from curr player, as it is included
			}

			// ask every player (except thrower) if they want to defend
			for (giocatoreT *defender = game_ctx->curr_player->next; !is_self(game_ctx, defender) && !blocked; defender = defender->next)
				if (target_defends(game_ctx, defender, card, effect))
					blocked = true;
			if (!blocked) { // no defense is used
				// iterate and apply effect from target (included) to game_ctx->curr_player (not included)
				do {
					apply_effect_target(game_ctx, effect, target);
					target = target->next;
				} while (!is_self(game_ctx, target));
			}
			break;
		}
	}
	return blocked;
}

/**
 * @brief applies all effects of a card during current player's turn (also asks user if wants to apply the effects if they are opzional)
 * 
 * @param game_ctx 
 * @param card card to apply effects from
 */
void apply_effects_now(game_contextT *game_ctx, cartaT *card) {
	bool apply = true, blocked = false;
	giocatoreT *target_tu = NULL;

	if (card->opzionale) {
		printf("Puoi attivare gli effetti opzionali di questa carta " COLORED_CARD_TYPE ":\n",
			tipo_cartaT_color(card->tipo),
			tipo_cartaT_str(card->tipo)
		);
		show_card(card);
		printf("Vuoi applicare gli effetti di questa carta? ");
		apply = ask_choice();
	}

	if (apply) {
		for (int i = 0; i < card->n_effetti && !blocked; i++) {
			if (apply_effect(game_ctx, card, &card->effetti[i], &target_tu)) {
				printf("La catena degli effetti di '%s' giocata da " PRETTY_USERNAME " e' stata interrotta!\n",
					card->name,
					game_ctx->curr_player->name
				);
				log_ss(game_ctx, "La catena degli effetti di '%s' giocata da %s e' stata interrotta.", card->name, game_ctx->curr_player->name);
				blocked = true; // stop executing further effects
			}
		}
	}
	if (blocked && (match_card_type(card, BONUS) || match_card_type(card, MALUS))) { // if bonus/malus card gets blocked it gets disposed
		leave_aula(game_ctx, game_ctx->curr_player, card, !DISPATCH_EFFECTS); // remove card from aula without activating leaving effects
		dispose_card(game_ctx, card);
	}
}

/**
 * @brief applies all effects of a card during current player's turn if supplied quando matches card's quando
 * 
 * @param game_ctx 
 * @param card card to apply effects from
 * @param quando time the card must match
 */
void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando) {
	if (card->quando == quando)
		apply_effects_now(game_ctx, card);
}


void apply_start_effects(game_contextT *game_ctx) {
	// TODO: add some logging in this function
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

	while (in_action) { // keep the menu open until action phase ends
		switch (choice_action_menu()) {
			case ACTION_PLAY_HAND: {
				if (play_card(game_ctx, ALL)) // check for successful play of card
					in_action = false; // end action phase
				break;
			}
			case ACTION_DRAW: {
				draw_card(game_ctx);
				in_action = false; // end action phase
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
				printf("Sei sicuro di volere uscire da questa partita? ");
				if (ask_choice()) {
					game_ctx->game_running = false;
					in_action = false; // end action phase
				}
				break;
			}
		}
	}
}

/**
 * @brief checks if the game can end because the current round player won
 * 
 * @param game_ctx 
 * @return true if current round player won
 * @return false if current round player didn't win
 */
bool check_win_condition(game_contextT *game_ctx) {
	bool can_win = has_bonusmalus(game_ctx->curr_player, INGEGNERE); // cant win with ingegnerizzazione
	int tot_students = count_cards_restricted(game_ctx->curr_player->aula, STUDENTE_SEMPLICE) +
		count_cards_restricted(game_ctx->curr_player->aula, LAUREANDO);
	return can_win && tot_students >= WIN_STUDENTS_COUNT;
}

void end_round(game_contextT *game_ctx) {
	if (!game_ctx->game_running)
		return;

	// hand max cards check
	while (count_cards(game_ctx->curr_player->carte) > ENDROUND_MAX_CARDS) {
		puts("Puoi avere massimo " ANSI_BOLD TO_STRING(ENDROUND_MAX_CARDS) ANSI_RESET " carte in mano alla fine del round!");
		discard_card(game_ctx, &game_ctx->curr_player->carte, ALL, "Carte attualmente nella tua mano");
	}

	if (check_win_condition(game_ctx)) { // check if curr player won
		printf("Congratulazioni " PRETTY_USERNAME ", hai vinto la partita!\n", game_ctx->curr_player->name);
		game_ctx->game_running = false; // stop game
	} else { // no win, keep playing
		printf("Round di " PRETTY_USERNAME " completato!\n", game_ctx->curr_player->name);
		game_ctx->curr_player = game_ctx->curr_player->next; // next round its next player's turn
		game_ctx->round_num++;
	}
}
