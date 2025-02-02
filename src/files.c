#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "structs.h"
#include "card.h"
#include "utils.h"

game_contextT* load_game() {
	// TODO: implement loading saved games
	return NULL;
}

void dump_effect(FILE *fp, effettoT *effect) {
	fprintf(fp, "%d %d %d\n", (int)effect->azione, (int)effect->target_giocatori, (int)effect->target_carta);
}

void dump_effects(FILE *fp, cartaT *card) {
	write_int(fp, card->n_effetti);
	for (int i = 0;  i < card->n_effetti; i++)
		dump_effect(fp, &card->effetti[i]);
}

void dump_card(FILE *fp, cartaT* card) {
	fprintf(fp, "%s\n", card->name);
	fprintf(fp, "%s\n", card->description);
	write_int(fp, (int)card->tipo);
	dump_effects(fp, card);
	write_int(fp, (int)card->quando);
	write_int(fp, (int)card->opzionale);
}

void dump_cards(FILE *fp, cartaT *head) {
	write_int(fp, count_cards(head));
	for (; head != NULL; head = head->next)
		dump_card(fp, head);
}

void dump_player(FILE *fp, giocatoreT *player) {
	fprintf(fp, "%s\n", player->name);
	dump_cards(fp, player->carte); // save mano
	dump_cards(fp, player->aula); // save aula
	dump_cards(fp, player->bonus_malus); // save bonus/malus
}

void save_game(game_contextT* game_ctx) {
	FILE *fp = fopen(FILE_SAVE, "w");
	if (fp == NULL) {
		fprintf(stderr, "Opening save file (%s) failed!\n", FILE_SAVE);
		exit(EXIT_FAILURE);
	}

	giocatoreT *player = game_ctx->curr_player;
	write_int(fp, game_ctx->n_players);
	for (int i = 0; i < game_ctx->n_players; i++, player = player->next)
		dump_player(fp, player);
	dump_cards(fp, game_ctx->mazzo_pesca); // save mazzo pesca
	dump_cards(fp, game_ctx->mazzo_scarti); // save mazzo scarti
	dump_cards(fp, game_ctx->aula_studio); // save aula studio
	fclose(fp);
}


effettoT* read_effetti(FILE* fp, int* n_effects) {
	int amount = read_int(fp);
	effettoT* effects = NULL;
	if (amount != 0)
		effects = (effettoT*)malloc_checked(amount*sizeof(effettoT));
	for (int i = 0; i < amount; i++) {
		effects[i].azione = (azioneT)read_int(fp);
		effects[i].target_giocatori = (target_giocatoriT)read_int(fp);
		effects[i].target_carta = (tipo_cartaT)read_int(fp);
	}
	*n_effects = amount;
	return effects;
}

// returns the new linked list tail
// returns (through amount pointer) the number of cards of this type present, sets *amount to 0 if no more cards are readable from fp
// takes a pointer to the current tail's next pointer
cartaT *read_carta(FILE *fp, cartaT **tail_next, int *amount) {
	if (fscanf(fp, "%d", amount) != 1) {
		*amount = 0;
		return NULL;
	}
	
	cartaT *card = (cartaT*)malloc_checked(sizeof(cartaT));

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

cartaT *load_mazzo(int *n_cards) {
	FILE* fp = fopen(FILE_MAZZO, "r");
	if (fp == NULL) {
		fprintf(stderr, "Opening cards file (%s) failed!\n", FILE_MAZZO);
		exit(EXIT_FAILURE);
	}

	*n_cards = 0;
	cartaT *mazzo = NULL, **tail_next = &mazzo, *new_tail;
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