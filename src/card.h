#ifndef CARD_H
#define CARD_H

#include <stdbool.h>
#include "types.h"

void clear_cards(cartaT *head);
cartaT *shuffle_cards(cartaT *cards, int n_cards);
cartaT *split_matricole(cartaT **mazzo_head);
cartaT *duplicate_carta(cartaT *card);
cartaT* pop_card(cartaT **head_ptr);
void push_card(cartaT **head_ptr, cartaT *card);
void unlink_card(cartaT **head_ptr, cartaT *card);
cartaT *card_by_index_restricted(cartaT *card, tipo_cartaT type, int idx);
int count_cards(cartaT *head);
bool match_card_type(cartaT *card, tipo_cartaT type);
bool cards_contain(cartaT *head, cartaT *needle);
bool cards_contain_specific(cartaT *head, cartaT *needle);
int count_cards_restricted(cartaT *head, tipo_cartaT type);

#endif // CARD_H