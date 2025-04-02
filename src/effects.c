#include "effects.h"
#include "structs.h"
#include "constants.h"
#include "format.h"
#include "logging.h"
#include "gameplay.h"
#include "card.h"
#include "utils.h"
#include "graphics.h"

/**
 * @brief applies ELIMINA effect on the given target.
 * this effect makes thrower pick which card to delete from targets's aula (aula + bonusmalus).
 * 
 * @param game_ctx current game state
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
 * @brief applies SCARTA effect on the given target.
 * this effect makes targets discard a card from their hands.
 * 
 * @param game_ctx current game state
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
		discarded_card = pick_random_card(target->carte, effect->target_carta);
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
 * @brief applies GIOCA effect on the given target.
 * this effect makes target play a card.
 * 
 * @param game_ctx current game state
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

		switch_player(game_ctx, target); // switch current player to the target player to create a sub-round for target to play a card
		if (!play_card(game_ctx, effect->target_carta)) {
			log_ss(game_ctx, "%s avrebbe dovuto giocare una carta %s, ma non ne aveva.",
				target->name,
				tipo_cartaT_str(effect->target_carta)
			);
		}
		switch_player(game_ctx, thrower); // switch back to original card thrower player
	}
}

/**
 * @brief applies RUBA effect on the given target.
 * this effect makes thrower pick which card to steal from targets's aula (aula + bonusmalus).
 * 
 * @param game_ctx current game state
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
			if (can_join_aula(game_ctx->curr_player, card))
				can_steal = true;
		}
	}

	if (can_steal) {
		do {
			card = pick_card(target_cards, effect->target_carta, prompt, title, ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET);

			if (can_join_aula(game_ctx->curr_player, card)) {
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
 * @brief applies PRENDI effect on the given target.
 * this effect extracts and removes a random card from the target player's hand putting it into the thrower player's hand.
 * 
 * @param game_ctx current game state
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
	stolen_card = pick_random_card(target->carte, effect->target_carta);
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
 * @brief applies PESCA effect on the given target.
 * this effect makes target draw a card.
 * 
 * @param game_ctx current game state
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
		switch_player(game_ctx, target); // switch current player to the target player to create a sub-round for target to draw a card
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
		switch_player(game_ctx, thrower); // switch back to original card thrower player
	}
}

/**
 * @brief applies SCAMBIA effect on the given target.
 * this effect swaps thrower and target player's hands.
 * 
 * @param game_ctx current game state
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

	printf(PRETTY_USERNAME " ha scambiato la sua mano con quella di " PRETTY_USERNAME " grazie all'effetto %s!\n",
		game_ctx->curr_player->name,
		target->name,
		azioneT_str(effect->azione)
	);
	log_sss(game_ctx, "%s scambia il suo mazzo con quello di %s grazie all'effetto %s.",
		game_ctx->curr_player->name,
		target->name,
		azioneT_str(effect->azione)
	);

	// swap hands
	game_ctx->curr_player->carte = target->carte;
	target->carte = thrower_cards;
}

/**
 * @brief this function just calls the given effect with the specified target
 * 
 * @param game_ctx current game state
 * @param effect effect to apply
 * @param target target player to apply effect on
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
 * @brief obtains the target player(s) of the effect and asks each of them if they want to defend. if target is TU, makes user select
 * the target_tu target for the current attack. calls apply_effect_target on each target. shows appropriate verbs for each action.
 * 
 * @param game_ctx current game state
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
 * @brief applies all effects of a card during current player's turn (also asks user if wants to apply the effects if they are optional)
 * 
 * @param game_ctx current game state
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
 * @param game_ctx current game state
 * @param card card to apply effects from
 * @param quando time the card must match
 */
void apply_effects(game_contextT *game_ctx, cartaT *card, quandoT quando) {
	if (card->quando == quando) {
		log_sss(game_ctx, "Applicazione degli effetti di '%s' (%s) di %s.",
			card->name,
			quandoT_str(card->quando),
			game_ctx->curr_player->name
		);
		apply_effects_now(game_ctx, card);
	}
}

/**
 * @brief this function applies start effects (quando = INIZIO) of aula cards of the current player.
 * particular caution avoiding bugs, as described in the multiline comment.
 * 
 * @param game_ctx current game state
 */
void apply_start_effects(game_contextT *game_ctx) {
	giocatoreT *player = game_ctx->curr_player;
	cartaT **aula_cards;
	int idx, n_cards = count_cards(player->bonus_malus) + count_cards(player->aula);

	if (n_cards == 0) // no cards in aula, no need to apply any effect
		return;

	/* 
	 * dump bonus_malus and aula cards in a dynamic array to apply their effects in sequence without using their linking logic
	 * to avoid issues like: applying effects of removed cards or applying effects of another list if the current card gets moved
	 * into another cards linked list during the applying its effects (like Sparacoriandoli using ELIMINA on itself).
	*/

	idx = 0;
	aula_cards = (cartaT**)malloc_checked(n_cards*sizeof(cartaT*));

	// dump bonus/malus cards first
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
