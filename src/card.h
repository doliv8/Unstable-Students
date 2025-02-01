#ifndef CARD_H
#define CARD_H

#include "types.h"

cartaT *shuffle_cards(cartaT *cards, int n_cards);
void clear_cards(cartaT *head);
cartaT *duplicate_carta(cartaT *);
cartaT* pop_card(cartaT **);
void push_card(cartaT **, cartaT *);
void unlink_card(cartaT **, cartaT *);
cartaT *card_by_index(cartaT *, int);

#endif // CARD_H