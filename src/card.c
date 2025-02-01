#include <stddef.h>
#include "card.h"
#include "structs.h"
#include "utils.h"

void clear_cards(cartaT *head) {
	// check for base case to keep recurse or not (freeing from tail to head)
	if (head->next)
		clear_cards(head->next);

	// clear actual card
	if (head->n_effetti != 0)
		free_wrap(head->effetti);
	free_wrap(head);
}

// this function uses Fisher-Yates shuffle algorithm to shuffle the (linearized) dynamic array of cards in linear time
cartaT *shuffle_cards(cartaT *cards, int n_cards) {
	if (n_cards == 0)
		return NULL;

	cartaT **linear_cards = (cartaT**)malloc_checked(n_cards*sizeof(cartaT*));

	for (int i = 0; i < n_cards; i++) {
		// pop the card from the head of the linked list
		linear_cards[i] = cards;
		cards = cards->next;
	}

	// actual Fisher-Yates shuffling algorithm
	cartaT *temp; // to hold temporary cartaT pointer for swapping cards
	for (int i = n_cards-1, j; i > 0; i--) {
		j = rand_int(0, i);
		// swap cards at index i and j
		temp = linear_cards[i];
		linear_cards[i] = linear_cards[j];
		linear_cards[j] = temp;
	}

	// reconstruct the links between the nodes (cards) of the linked list following the shuffled order
	for (int i = 0; i < n_cards-1; i++)
		linear_cards[i]->next = linear_cards[i+1];
	linear_cards[n_cards-1]->next = NULL; // set tail next pointer to NULL

	cartaT *new_head =  linear_cards[0];

	// free the linear array of cards used during shuffle
	free_wrap(linear_cards);

	return new_head; // return new head to shuffled linked list
}

cartaT *duplicate_carta(cartaT *card) {
	cartaT *copy_card = (cartaT*)malloc_checked(sizeof(cartaT));
	*copy_card = *card; // copy the whole struct
	if (card->n_effetti != 0) {
		// create new effects array
		effettoT* effects = (effettoT*)malloc_checked(card->n_effetti*sizeof(effettoT));
		// copy effects into the new array
		for (int i = 0; i < card->n_effetti; i++)
			effects[i] = card->effetti[i];
		copy_card->effetti = effects;
	}
	return copy_card;
}

cartaT* pop_card(cartaT** head_ptr) {
	cartaT* card = *head_ptr;
	if (card == NULL)
		return NULL;
	*head_ptr = card->next;
	card->next = NULL;
	return card;
}

void push_card(cartaT** head_ptr, cartaT* card) {
	card->next = *head_ptr;
	*head_ptr = card;
}

void unlink_card(cartaT **head_ptr, cartaT *card) {
	while (*head_ptr != NULL && *head_ptr != card)
		head_ptr = &(*head_ptr)->next;
	pop_card(head_ptr);
}

// idx is 1 indexed
cartaT *card_by_index(cartaT *head, int idx) {
	while (head != NULL && --idx > 0)
		head = head->next;
	return head;
}

int count_cards(cartaT *head) {
	int count;
	for (count = 0; head != NULL; head = head->next)
		count++;
	return count;
}