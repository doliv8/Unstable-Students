#include "effects.h"
#include "structs.h"
#include "constants.h"
#include "format.h"
#include "logging.h"
#include "gameplay.h"
#include "card.h"
#include "utils.h"

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
