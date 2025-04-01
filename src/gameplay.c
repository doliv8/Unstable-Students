#include <stdio.h>
#include <string.h>
#include "gameplay.h"
#include "graphics.h"
#include "format.h"
#include "structs.h"
#include "card.h"
#include "files.h"
#include "logging.h"
#include "utils.h"
#include "effects.h"
#include "stats.h"

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

/**
 * @brief switch current round's player
 * 
 * @param game_ctx 
 * @param player player to switch to
 */
void switch_player(game_contextT *game_ctx, giocatoreT *player) {
	bool found = false;
	player_statsT *stats = game_ctx->curr_stats;

	game_ctx->curr_player = player; // switch player

	// find stats of the given player (need to switch those aswell)
	for (int i = 0; i < game_ctx->n_players && !found; i++, stats = stats->next) {
		if (!strncmp(stats->name, player->name, GIOCATORE_NAME_LEN)) {
			game_ctx->curr_stats = stats;
			found = true;
		}
	}
}

/**
 * @brief checks if a player has a card with the given action in his bonus/malus cards
 * 
 * @param player player to run the check on
 * @param effect_action effect action to look for
 * @return true if player has a card with an effect_action effect
 * @return false if player doesn't have a card with an effect_action effect
 */
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
 * @brief checks if a player has a card with the given action targeting the given target card in his bonus/malus cards
 * 
 * @param player player to run the check on
 * @param effect_action effect action to look for
 * @param target card target to look for
 * @return true if player has a card with an effect_action effect targeting target card
 * @return false if player doesn't have a card with an effect_action effect targeting target card
 */
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
			defense_card = pick_card(target->carte, ISTANTANEA, prompt, "Istantanee nella tua mano", ANSI_BLUE "%s" ANSI_RESET);
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
	
		switch_player(game_ctx, target); // switch current player to defending player for applying defense card effects correctly
		apply_effects(game_ctx, defense_card, SUBITO); // appply additional defense card effects
		stats_add_played_card(game_ctx, defense_card);
		switch_player(game_ctx, attacker); // switch back to attacking player

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
 * @param player target player
 */
void show_player_state(giocatoreT *player) {
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
			show_player_state(player);
	} else
		show_player_state(target);
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
 * @brief prompts user to pick a card from the provided cards list with a type restriction filter and returns the picked card
 * (still chained in the list).
 * 
 * @param head cards list
 * @param type card type user is allowed to pick
 * @param prompt text shown to the user while asked to pick the card
 * @param title title of the group of cards (used to calculate visual length of title), can't contain colors
 * @param title_fmt title format string (must always contain one and only one %s and no other formatters), can contain colors
 * @return cartaT* pointer to picked card or NULL if there's no card to pick
 */
cartaT *pick_card(cartaT *head, tipo_cartaT type, const char *prompt, const char *title, const char *title_fmt) {
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
 * @brief automatically picks a card from the provided cards list with a type restriction filter and returns the picked card
 * (still chained in the list).
 * 
 * @param head cards list
 * @param type card type allowed to be extracted
 * @return cartaT* pointer to picked card or NULL if there's no card to extract
 */
cartaT *pick_random_card(cartaT *head, tipo_cartaT type) {
	cartaT *card;
	int n_cards = count_cards_restricted(head, type), chosen_idx;

	if (n_cards != 0) {
		chosen_idx = rand_int(1, n_cards); // pick a 1-indexed index of the restricted cards
		card = card_by_index_restricted(head, type, chosen_idx);
	} else {
		printf("Non ci sono carte " COLORED_CARD_TYPE " da estrarre!\n",
			tipo_cartaT_color(type),
			tipo_cartaT_str(type)
		);
		card = NULL;
	}
	return card;
}

/**
 * @brief this function is similar to pick_card but specific to picking a card from a player's aula (bonus_malus + aula).
 * if specified type allows both BONUS/MALUS and STUDENT (is ALL) and there are cards both in bonus_malus and aula, asks
 * user if wants to pick from aula or bonus_malus and then calls actual pick_card on the chosen list. otherwise, pick_card
 * is directly called on the only available chosable list (bonus_malus or aula).
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
		card = pick_card(target->aula, type, prompt, aula_title, ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
	} else if (n_aula == 0) { // only bonus/malus has cards
		card = pick_card(target->bonus_malus, type, prompt, bonusmalus_title, ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
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
			card = pick_card(target->aula, type, prompt, aula_title, ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET);
		else // choice was bonus/malus
			card = pick_card(target->bonus_malus, type, prompt, bonusmalus_title, ANSI_BOLD ANSI_MAGENTA "%s" ANSI_RESET);
	}
	free_wrap(bonusmalus_title);
	free_wrap(aula_title);
	return card;
}

/**
 * @brief puts the given card in aula studio if card is MATRICOLA-kind, or in mazzo scarti otherwise.
 * 
 * @param game_ctx 
 * @param card card to dispose
 */
void dispose_card(game_contextT *game_ctx, cartaT *card) {
	if (match_card_type(card, MATRICOLA))
		push_card(&game_ctx->aula_studio, card); // put MATRICOLA into aula studio
	else
		push_card(&game_ctx->mazzo_scarti, card); // put card into mazzo scarti
}

/**
 * @brief asks current player to discard a card of the specified kind from the given cards list
 * 
 * @param game_ctx 
 * @param cards pointer to list of cards to discard from
 * @param type type of card to discard
 * @param title title shown while picking the card to discard
 */
void discard_card(game_contextT *game_ctx, cartaT **cards, tipo_cartaT type, const char *title) {
	cartaT *card = pick_card(*cards, type, "Scegli la carta che vuoi scartare.", title, ANSI_BOLD ANSI_RED "%s" ANSI_RESET);

	if (card != NULL) {
		unlink_card(cards, card);
		dispose_card(game_ctx, card); // dispose discarded card
		printf("Hai scartato: %s\n", card->name);
		log_ss(game_ctx, "%s ha scartato '%s'.", game_ctx->curr_player->name, card->name);
		stats_add_discarded(game_ctx);
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

/**
 * @brief calculates the amount of playable cards (of the specified type) from the hand of the current player
 * 
 * @param game_ctx 
 * @param type type of cards
 * @return int amount of playable cards
 */
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

	card = pick_card(thrower->carte, type, playable_prompt,
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
				if (can_join_aula(target, card)) {
					log_sss(game_ctx, "%s gioca '%s' su %s.", thrower->name, card->name, target->name);
					unlink_card(&thrower->carte, card);
					if (target == thrower || !target_defends(game_ctx, target, card, CARD_PLACEMENT)) // can't defended from self thrown cards
						join_aula(game_ctx, target, card);
					else
						dispose_card(game_ctx, card);
					stats_add_played_card(game_ctx, card);
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
						stats_add_played_card(game_ctx, card);
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
				stats_add_played_card(game_ctx, card);
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
int choice_action_menu(void) {
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
		switch_player(game_ctx, player);
		// apply leave effects
		apply_effects(game_ctx, card, FINE);
		switch_player(game_ctx, original_player);
	}
}

/**
 * @brief checks if card can join aula (no equal card is already in aula)
 * 
 * @param player target aula player
 * @param card joining card
 * @return true if card can join aula
 * @return false if card couldn't join aula
 */
bool can_join_aula(giocatoreT *player, cartaT *card) {
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
 * @brief checks if the game can end because the current round player won
 * 
 * @param game_ctx 
 * @return true if current round player won
 * @return false if current round player didn't win
 */
bool check_win_condition(game_contextT *game_ctx) {
	bool can_win = !has_bonusmalus(game_ctx->curr_player, INGEGNERE); // cant win with ingegnerizzazione
	int tot_students = count_cards_restricted(game_ctx->curr_player->aula, STUDENTE);
	return can_win && tot_students >= WIN_STUDENTS_COUNT;
}

/**
 * @brief first phase of the round
 * 
 * @param game_ctx 
 */
void begin_round(game_contextT *game_ctx) {
	save_game(game_ctx);

	show_round(game_ctx);

	apply_start_effects(game_ctx);

	draw_card(game_ctx);
}

/**
 * @brief second phase of the round
 * 
 * @param game_ctx 
 */
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
 * @brief third (and last) phase of the round
 * 
 * @param game_ctx 
 */
void end_round(game_contextT *game_ctx) {
	if (!game_ctx->game_running)
		return;

	// hand max cards check
	while (count_cards(game_ctx->curr_player->carte) > ENDROUND_MAX_CARDS) {
		puts("Puoi avere massimo " ANSI_BOLD TO_STRING(ENDROUND_MAX_CARDS) ANSI_RESET " carte in mano alla fine del round!");
		discard_card(game_ctx, &game_ctx->curr_player->carte, ALL, "Carte attualmente nella tua mano");
	}

	stats_add_round(game_ctx);

	if (check_win_condition(game_ctx)) { // check if curr player won
		printf(ANSI_CYAN "\nCongratulazioni " ANSI_RED ANSI_BOLD PRETTY_USERNAME ANSI_CYAN ", hai vinto la partita!\n\n" ANSI_RESET, game_ctx->curr_player->name);
		puts(WIN_ASCII_ART);
		log_s(game_ctx, "%s ha vinto la partita!", game_ctx->curr_player->name);
		stats_add_win(game_ctx);
		game_ctx->game_running = false; // stop game
	} else { // no win, keep playing
		printf("\nRound di " PRETTY_USERNAME " completato!\n", game_ctx->curr_player->name);
		switch_player(game_ctx, game_ctx->curr_player->next); // next round its next player's turn
		game_ctx->round_num++;
	}

	save_stats(game_ctx);
}
