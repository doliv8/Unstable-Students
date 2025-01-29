// Nome: Diego Oliva (d.oliva@studenti.unica.it)
// Matricola: 60/61/66678
// Tipologia progetto: avanzato

#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "types.h"
#include "structs.h"
#include "utils.h"
#include "gameplay.h"

cartaT* duplicate_carta(cartaT* card) {
	cartaT* copy_card = (cartaT*)malloc_checked(sizeof(cartaT));
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
cartaT* read_carta(FILE* fp, cartaT** tail_next, int* amount) {
	if (fscanf(fp, "%d", amount) != 1) {
		*amount = 0;
		return NULL;
	}
	
	cartaT* card = (cartaT*)malloc_checked(sizeof(cartaT));

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

cartaT* load_mazzo(int* n_cards) {
	FILE* fp = fopen(FILE_MAZZO, "r");
	if (fp == NULL) {
		fprintf(stderr, "Opening cards file (%s) failed!\n", FILE_MAZZO);
		exit(EXIT_FAILURE);
	}

	*n_cards = 0;
	cartaT* mazzo = NULL, **tail_next = &mazzo, *new_tail;
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

// this function uses Fisher-Yates shuffle algorithm to shuffle the (linearized) dynamic array of cards in linear time
cartaT* shuffle_cards(cartaT* cards, int n_cards) {
	if (n_cards == 0)
		return NULL;

	cartaT** linear_cards = malloc_checked(n_cards*sizeof(cartaT*));

	for (int i = 0; i < n_cards; i++) {
		// pop the card from the head of the linked list
		linear_cards[i] = cards;
		cards = cards->next;
	}

	// actual Fisher-Yates shuffling algorithm
	cartaT* temp; // to hold temporary cartaT pointer for swapping cards
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

	cartaT* new_head =  linear_cards[0];

	// free the linear array of cards used during shuffle
	free(linear_cards);

	return new_head; // return new head to shuffled linked list
}

cartaT* pop_card(cartaT** head) {
	cartaT* card = *head;
	if (card == NULL)
		return NULL;
	*head = card->next;
	card->next = NULL;
	return card;
}

void push_card(cartaT** head, cartaT* card) {
	card->next = *head;
	*head = card;
}

// returns a linked list containing all the Matricola-kind cards found & removed from mazzo
cartaT* split_matricole(cartaT** mazzo_head) {
	cartaT* matricole_head = NULL;
	for (cartaT** prev = mazzo_head, *curr; *prev != NULL; ) {
		curr = *prev;
		if (curr->tipo == MATRICOLA) {
			// remove curr card from the mazzo linked list linking
			pop_card(prev);
			// link this matricola card to matricole_head linked_list
			push_card(&matricole_head, curr);
		} else
			prev = &curr->next;
	}

	return matricole_head;
}

giocatoreT* new_player() {
	giocatoreT* player = (giocatoreT*)calloc_checked(1, sizeof(giocatoreT));

	do {
		printf("Inserisci il nome del giocatore: ");
		scanf(" %" TO_STRING(GIOCATORE_NAME_LEN) "[^\n]", player->name);
	} while (!strnlen(player->name, sizeof(player->name)));

	return player;
}

void distribute_cards(game_contextT* game_ctx) {
	cartaT* card;

	// distribute a matricola card for each player, rotating player for each given card
	for (int i = 0; i < game_ctx->n_players; i++, game_ctx->next_player = game_ctx->next_player->next) {
		card = pop_card(&game_ctx->aula_studio);
		push_card(&game_ctx->next_player->aula, card);
	}

	// distribute a card CARDS_PER_PLAYER times the players count, rotating player for each given card
	for (int i = 0; i < CARDS_PER_PLAYER * game_ctx->n_players; i++, game_ctx->next_player = game_ctx->next_player->next) {
		card = pop_card(&game_ctx->mazzo_pesca);
		push_card(&game_ctx->next_player->carte, card);
	}
}

game_contextT* new_game() {
	game_contextT* game_ctx = (game_contextT*)calloc_checked(1, sizeof(game_contextT));

	do {
		puts("Quanti giocatori giocheranno?");
		game_ctx->n_players = get_int();
	} while (game_ctx->n_players < MIN_PLAYERS || game_ctx->n_players > MAX_PLAYERS);

	// create players
	// game_ctx->next_player serves as the linked-list head
	giocatoreT* curr_player;
	for (int i = 0; i < game_ctx->n_players; i++) {
		if (game_ctx->next_player == NULL)
			curr_player = game_ctx->next_player = new_player();
		else
			curr_player = curr_player->next = new_player();
	}
	curr_player->next = game_ctx->next_player; // make the linked list circular linking tail to head

	// load cards
	int n_cards;
	cartaT* mazzo = load_mazzo(&n_cards);
	mazzo = shuffle_cards(mazzo, n_cards);

	game_ctx->aula_studio = split_matricole(&mazzo);
	game_ctx->mazzo_pesca = mazzo;

	distribute_cards(game_ctx);

	game_ctx->round_num = 1;

	return game_ctx;
}

game_contextT* load_game() {
	// TODO: implement loading saved games
	return NULL;
}

void save_game(game_contextT* game_ctx) {
	// TODO: implement saving game
}

void free_cards(cartaT* cards_head) {
	// check for base case to keep recurse or not (freeing from tail to head)
	if (cards_head->next)
		free_cards(cards_head->next);

	// clear actual card
	if (cards_head->n_effetti != 0)
		free_wrap(cards_head->effetti);
	free_wrap(cards_head);
}

// recursive function to clear a giocatoreT* circular linked list
void clear_players(giocatoreT* head, giocatoreT* p) {
	// check for base case to recurse or not
	if (p->next != head)
		clear_players(head, p->next);

	// clear actual player
	if (p->aula != NULL)
		free_cards(p->aula);
	if (p->bonus_malus != NULL)
		free_cards(p->bonus_malus);
	if (p->carte != NULL)
		free_cards(p->carte);
	free_wrap(p);
}

void clear_game(game_contextT* game_ctx) {
	clear_players(game_ctx->next_player, game_ctx->next_player);

	if (game_ctx->aula_studio != NULL)
		free_cards(game_ctx->aula_studio);
	if (game_ctx->mazzo_pesca != NULL)
		free_cards(game_ctx->mazzo_pesca);
	if (game_ctx->mazzo_scarti != NULL)
		free_cards(game_ctx->mazzo_scarti);
	free_wrap(game_ctx);
}


void apply_effects(game_contextT* game_ctx, cartaT* card) {

	if (card->opzionale) {
		printf("Vuoi applicare gli effetti della carta %s? (y/n): ", card->name);
		// TODO: implement choice
	}

	for (int i = 0; i < card->n_effetti; i++) {
		printf("*to implement* APPLYING an effect!\n");
		// TODO: apply actual effects
		// apply_effect(game_ctx, card->effetti[i]);
	}
}


void show_round(game_contextT* game_ctx) {
	printf("Round numero: %d\n", game_ctx->round_num);

	printf("Ora gioca: %s\n", game_ctx->next_player->name);


}

void apply_start_effects(game_contextT* game_ctx) {
	giocatoreT* player = game_ctx->next_player;
	// apply bonus malus quando = INIZIO effects
	for (cartaT* card = player->bonus_malus; card != NULL; card = card->next) {
		if (card->quando == INIZIO)
			apply_effects(game_ctx, card);
	}

	// apply aula quando = INIZIO effects
	for (cartaT* card = player->aula; card != NULL; card = card->next) {
		if (card->quando == INIZIO)
			apply_effects(game_ctx, card);
	}

}

int count_cards(cartaT* head) {
	int count = 0;
	while (head != NULL) {
		count++;
		head = head->next;
	}
	return count;
}

void draw_card(game_contextT* game_ctx) {
	// shuffle and swap mazzo_scarti with mazzo_pesca if mazzo_pesca is empty
	if (game_ctx->mazzo_pesca == NULL) {
		game_ctx->mazzo_pesca = shuffle_cards(game_ctx->mazzo_scarti, count_cards(game_ctx->mazzo_scarti));
		game_ctx->mazzo_pesca = game_ctx->mazzo_scarti;
		game_ctx->mazzo_scarti = NULL; // mazzo_scarti has been moved to mazzo_pesca (emptied)
	}

	cartaT* drawn_card = pop_card(&game_ctx->mazzo_pesca);
	puts("Ecco la carta che hai pescato:");
	show_card(drawn_card);
	push_card(&game_ctx->next_player->carte, drawn_card);
}

void format_effects(freeable_multiline_textT* multiline, cartaT* card) {
	char* line;
	if (card->n_effetti != 0) {
		// add upper padding
		for (int i = 0; i < MAX_EFFECTS-card->n_effetti; i++)
			multiline_addline(multiline, strdup_checked(""));

		asprintf_checked(&line, "Opzionale: %s", card->opzionale ? "Si" : "No");
		multiline_addline(multiline, line);
		asprintf_checked(&line, "Quando: %s", quandoT_str(card->quando));
		multiline_addline(multiline, line);
		asprintf_checked(&line, "Effetti (%d):", card->n_effetti);
		multiline_addline(multiline, line);

		// add actual effects
		for (int i = 0; i < card->n_effetti; i++) {
			asprintf_checked(&line, "%s -> %s (%s)",
				azioneT_str(card->effetti[i].azione),
				tipo_cartaT_str(card->effetti[i].target_carta),
				target_giocatoriT_str(card->effetti[i].target_giocatori)
			);
			multiline_addline(multiline, line);
		}
	} else {
		for (int i = 1; i < CARD_EFFECTS_HEIGHT; i++)
			multiline_addline(multiline, strdup_checked(""));
		multiline_addline(multiline, strdup_checked("Nessun effetto!"));
	}
}

void show_card(cartaT *card) {
	freeable_multiline_textT card_info;
	init_multiline(&card_info);
	build_card(&card_info, card);
	for (int i = 0; i < card_info.n_lines; i++)
		puts(card_info.lines[i]);
	clear_freeable_multiline(&card_info);
}

void build_card(freeable_multiline_textT *multiline, cartaT *card) {
	char *h_border, *v_border, *fmt_name, *type, *fmt_type;
	int len_name, len_type;
	wrapped_textT wrapped_description;
	freeable_multiline_textT effects_lines;
	init_multiline(&effects_lines);

	asprintf_checked(&h_border, ANSI_BLUE "%c%s%c" ANSI_RESET, CARD_CORNER_LEFT, HORIZONTAL_BAR, CARD_CORNER_RIGHT);
	asprintf_checked(&v_border, ANSI_BLUE "%c" ANSI_RESET, CARD_BORDER_VERTICAL);

	len_name = strlen(card->name);
	asprintf_checked(&fmt_name, ANSI_BOLD "%s" ANSI_RESET, card->name);

	len_type = asprintf_checked(&type, "#%s", tipo_cartaT_str(card->tipo));
	asprintf_checked(&fmt_type, ANSI_BG_GREEN "%s" ANSI_RESET, type);

	// compute wrapped description
	init_wrapped(&wrapped_description, card->description, CARD_CONTENT_WIDTH-CARD_PADDING);

	// compute effects
	format_effects(&effects_lines, card);

	// add all lines now
	// apend upper border
	multiline_addline(multiline, h_border);
	// append type
	multiline_addline(multiline, center_boxed_string(fmt_type, len_type, v_border, CARD_CONTENT_WIDTH));
	// append name
	multiline_addline(multiline, center_boxed_string(fmt_name, len_name, v_border, CARD_CONTENT_WIDTH));
	// now append wrapped description
	for (int i = 0; i < wrapped_description.multiline.n_lines; i++) {
		multiline_addline(multiline, center_boxed_string(
			wrapped_description.multiline.lines[i],
			wrapped_description.multiline.lengths[i],
			v_border,
			CARD_CONTENT_WIDTH
		)); // add centered boxed line
	}
	// append spacing for reserved description height
	for (int i = 0; i < CARD_DESCRIPTION_HEIGHT-wrapped_description.multiline.n_lines; i++)
		multiline_addline(multiline, center_boxed_string("", 0, v_border, CARD_CONTENT_WIDTH));
	// now append effects
	for (int i = 0; i < effects_lines.n_lines; i++) {
		multiline_addline(multiline, center_boxed_string(
			effects_lines.lines[i],
			effects_lines.lengths[i],
			v_border,
			CARD_CONTENT_WIDTH
		)); // add centered boxed line
	}
	// append bottom border
	multiline_addline(multiline, strdup_checked(h_border)); // duplicate the existing h_border into the heap

	free_wrap(v_border);
	free_wrap(fmt_name);
	free_wrap(type);
	free_wrap(fmt_type);
	clear_wrapped(&wrapped_description);
	clear_freeable_multiline(&effects_lines);
}

void show_cards(cartaT *head) {
	multiline_containerT container;
	init_multiline_container(&container);

	int count = count_cards(head);

	// TODO: implement containers for printing cards in a horizontal row



 
	clear_multiline_container(&container);
}

void begin_round(game_contextT* game_ctx) {
	show_round(game_ctx);
	apply_start_effects(game_ctx);

	draw_card(game_ctx);

}

void discard_card(game_contextT* game_ctx) {
	puts("You can have a maxium of " TO_STRING(ENDROUND_MAX_CARDS) " at the end of each round!");
	puts("Pick a card you want to discard:");
	cartaT* cards = game_ctx->next_player->carte;
	// for (int i = 1; < )
}

void end_round(game_contextT* game_ctx) {
	game_ctx->next_player = game_ctx->next_player->next; // next round its next player's turn
	game_ctx->round_num++;

	// hand max cards check
	// while (count_cards(game_ctx->next_player->carte) > ENDROUND_MAX_CARDS)
	// 	discard_card(game_ctx);

	// check win condition
}

int choice_action_menu() {
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

void show_player_state(game_contextT* game_ctx, giocatoreT* player) {
	// TODO: implement this function
	printf("Ecco lo stato di %s:\n", player->name);
	// keep in mind MOSTRA effect
}

void view_others(game_contextT* game_ctx) {
	giocatoreT* player;
	int chosen_idx;
	do {
		puts("Scegli il giocatore del quale vuoi vedere lo stato:");
		player = game_ctx->next_player->next; // start from next player based on turns
		for (int i = 1; i < game_ctx->n_players; i++, player = player->next)
			printf(" [TASTO %d] %s\n", i, player->name);
		printf(" [TASTO %d] Tutti i giocatori\n", game_ctx->n_players);
		chosen_idx = get_int();
	} while (chosen_idx < 1 || chosen_idx > game_ctx->n_players);

	player = game_ctx->next_player->next; // start from next player based on turns
	for (int i = 1; i < game_ctx->n_players; i++, player = player->next) {
		if (chosen_idx == game_ctx->n_players || i == chosen_idx)
			show_player_state(game_ctx, player);
	}
}

// returns true if user wants to quit, false otherwise
bool ask_quit() {
	char choice = 'n';
	printf("Are you sure you want to quit this game? (y/N): ");
	scanf(" %c", &choice);
	return tolower(choice) == 'y';
}

void play_round(game_contextT* game_ctx) {
	bool in_action = true;
	while (in_action) {
		switch (choice_action_menu()) {
			case ACTION_PLAY_HAND: {
				// TODO: implement playing cards
				in_action = false;
				break;
			}
			case ACTION_DRAW: {
				draw_card(game_ctx);
				in_action = false;
				break;
			}
			case ACTION_VIEW_OWN: {
				// TODO: implement viewing own cards
				break;
			}
			case ACTION_VIEW_OTHERS: {
				view_others(game_ctx);
				break;
			}
			case ACTION_QUIT: {
				if (ask_quit()) {
					game_ctx->game_running = false;
					in_action = false;
				}
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	// seed libc random generator
	srand(time(NULL));
	
	// check salvataggio
	assert(argc == 1);

	game_contextT* game_ctx = new_game();

	// game loop
	game_ctx->game_running = true;
	while (game_ctx->game_running) {
		save_game(game_ctx);

		begin_round(game_ctx);

		play_round(game_ctx);

		end_round(game_ctx);
	}



	clear_game(game_ctx);
	
}