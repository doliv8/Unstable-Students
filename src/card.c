#include <stddef.h>
#include <string.h>
#include "card.h"
#include "structs.h"
#include "utils.h"

/**
 * @brief free a linked list of cards recursively
 * 
 * @param head cards list
 */
void clear_cards(cartaT *head) {
	// check for base case to keep recurse or not (freeing from tail to head)
	if (head->next)
		clear_cards(head->next);

	// clear actual card
	if (head->n_effetti != 0)
		free_wrap(head->effetti);
	free_wrap(head);
}

/**
 * @brief this function uses Fisher-Yates shuffle algorithm to shuffle the (linearized) dynamic array of cards in linear time
 * 
 * @param cards linked-list of cards to shuffle
 * @return cartaT* shuffled cards linked-list
 */
cartaT *shuffle_cards(cartaT *cards) {
	cartaT **linear_cards, *new_head;
	cartaT *temp; // to hold temporary cartaT pointer for swapping cards
	int n_cards = count_cards(cards);

	if (n_cards == 0)
		return NULL;

	linear_cards = (cartaT**)malloc_checked(n_cards*sizeof(cartaT*));

	for (int i = 0; i < n_cards; i++) {
		// pop the card from the head of the linked list
		linear_cards[i] = cards;
		cards = cards->next;
	}

	// actual Fisher-Yates shuffling algorithm
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

	new_head = linear_cards[0];

	// free the linear array of cards used during shuffle
	free_wrap(linear_cards);

	return new_head; // return new head to shuffled linked list
}

/**
 * @brief returns a linked list containing all the Matricola-kind cards found and removed from provided mazzo
 * 
 * @param mazzo_head pointer to head of mazzo linked list
 * @return cartaT* linked list of all the MATRICOLA found and removed from mazzo
 */
cartaT *split_matricole(cartaT **mazzo_head) {
	cartaT *matricole_head = NULL;

	for (cartaT **prev = mazzo_head, *curr; *prev != NULL; ) {
		curr = *prev;
		if (match_card_type(curr, MATRICOLA)) {
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
 * @brief duplicates a given card creating another heap-allocated card creating a new copy of the effects aswell
 * 
 * @param card card to make a copy of
 * @return cartaT* card copy
 */
cartaT *duplicate_carta(cartaT *card) {
	cartaT *copy_card = (cartaT*)malloc_checked(sizeof(cartaT));

	*copy_card = *card; // copy the whole struct
	if (card->n_effetti != 0) {
		// create new effects array
		copy_card->effetti = (effettoT*)malloc_checked(card->n_effetti*sizeof(effettoT));
		// copy effects into the new array
		for (int i = 0; i < card->n_effetti; i++)
			copy_card->effetti[i] = card->effetti[i];
	}

	return copy_card;
}

/**
 * @brief pop a card from the head of a cards linked-list
 * 
 * @param head_ptr pointer to head of the linked-list
 * @return cartaT* popped card
 */
cartaT *pop_card(cartaT **head_ptr) {
	cartaT *card = *head_ptr;

	if (card == NULL)
		return NULL;

	*head_ptr = card->next;
	card->next = NULL;

	return card;
}

/**
 * @brief push a card to the head of a linked-list
 * 
 * @param head_ptr pointer to head of the linked-list
 * @param card card to insert in the linked-list
 */
void push_card(cartaT **head_ptr, cartaT *card) {
	card->next = *head_ptr;
	*head_ptr = card;
}

/**
 * @brief remove a card from inside a linked list (keeping linked-list links valid)
 * 
 * @param head_ptr pointer to head of the linked-list
 * @param card card to remove from the linked-list
 */
void unlink_card(cartaT **head_ptr, cartaT *card) {
	while (*head_ptr != NULL && *head_ptr != card)
		head_ptr = &(*head_ptr)->next;
	pop_card(head_ptr);
}

/**
 * @brief returns a card element from the provided list indexed by idx
 * 
 * @param head card list
 * @param type card type filter
 * @param idx 1-indexed index of card from provided card list
 * @return cartaT* card indexed or NULL if index was out of bounds (card not found)
 */
cartaT *card_by_index_restricted(cartaT *head, tipo_cartaT type, int idx) {
	cartaT *target = NULL;

	for (; head != NULL && target == NULL; head = head->next) {
		if (match_card_type(head, type) && --idx == 0)
			target = head;
	}

	return target;
}

/**
 * @brief checks if a card matches a given card type (card type wildcards supported)
 * 
 * @param card card to run type matching check on
 * @param type type of card to check against
 * @return true if card matches the given card type
 * @return false if card doesn't match the given card type
 */
bool match_card_type(cartaT *card, tipo_cartaT type) {
	bool matched;

	switch (type) {
		case ALL: {
			matched = true;
			break;
		}
		case STUDENTE: {
			matched = card->tipo == MATRICOLA || card->tipo == STUDENTE_SEMPLICE || card->tipo == LAUREANDO;
			break;
		}
		default: {
			matched = card->tipo == type;
			break;
		}
	}

	return matched;
}

/**
 * @brief checks if two given cards are equal comparing their names
 * 
 * @param first first card
 * @param second second card
 * @return true if cards are considered equal
 * @return false if cards are not equal
 */
bool cards_equal(cartaT *first, cartaT *second) {
	// only check for name matching
	return !strncmp(first->name, second->name, CARTA_NAME_LEN);
}

/**
 * @brief checks if a cards linked-list contains a card with the same name as the given card
 * 
 * @param head list to search needle in
 * @param needle card to search for
 * @return true if cards linked-list contain a card with the same name as the given one
 * @return false if cards linked-list doesn't contain the given card
 */
bool cards_contain(cartaT *head, cartaT *needle) {
	bool contained = false;

	for (; head != NULL && !contained; head = head->next)
		contained = cards_equal(head, needle);

	return contained;
}

/**
 * @brief checks if a cards linked-list contains the specific given card, by heap-address comparison
 * 
 * @param head list to search needle in
 * @param needle card to search for
 * @return true if cards linked-list contain the given card
 * @return false if cards linked-list doesn't contain the given card
 */
bool cards_contain_specific(cartaT *head, cartaT *needle) {
	bool contained = false;

	for (; head != NULL && !contained; head = head->next)
		contained = head == needle;

	return contained;
}

/**
 * @brief count the cards contained in a linked-list
 * 
 * @param head head of the linked-list
 * @return int count of cards present in the given linked-list
 */
int count_cards(cartaT *head) {
	int count;

	for (count = 0; head != NULL; head = head->next)
		count++;

	return count;
}

/**
 * @brief count the cards contained in a linked-list with type restriction
 * 
 * @param head head of the linked-list
 * @param type type restriction to filter the count with
 * @return int count of cards present in the given linked-list
 */
int count_cards_restricted(cartaT *head, tipo_cartaT type) {
	int count;

	for (count = 0; head != NULL; head = head->next) {
		if (match_card_type(head, type))
			count++;
	}

	return count;
}