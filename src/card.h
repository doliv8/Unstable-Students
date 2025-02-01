#ifndef CARD_H
#define CARD_H

#include "types.h"

void clear_cards(cartaT *head);
cartaT *shuffle_cards(cartaT *cards, int n_cards);
cartaT *duplicate_carta(cartaT *card);
cartaT* pop_card(cartaT **head_ptr);
void push_card(cartaT **head_ptr, cartaT *card);
void unlink_card(cartaT **head_ptr, cartaT *card);
cartaT *card_by_index(cartaT *card, int idx);
int count_cards(cartaT *head);

#endif // CARD_H