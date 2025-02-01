#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "structs.h"
#include "card.h"
#include "utils.h"

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