#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "card.h"
#include "utils.h"
#include "logging.h"


void file_read_failed() {
	fputs("Error occurred while reading from a file stream!\n", stderr);
	exit(EXIT_FAILURE);
}

void file_write_failed() {
	fputs("Error occurred while writing to a file stream!\n", stderr);
	exit(EXIT_FAILURE);
}

effettoT *load_effects(FILE *fp, int amount) {
	effettoT *effects = NULL;

	if (amount > 0) {
		effects = (effettoT*)malloc_checked(amount*sizeof(effettoT));
		if (fread(effects, sizeof(effettoT), amount, fp) != amount)
			file_read_failed();
	}
	return effects;
}

cartaT *load_card(FILE *fp) {
	cartaT *card;

	card = (cartaT*)malloc_checked(sizeof(cartaT));

	if (fread(card, sizeof(cartaT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_read_failed();

	card->effetti = load_effects(fp, card->n_effetti);
	return card;
}

cartaT *load_cards(FILE *fp) {
	int n_cards;
	cartaT *head = NULL, *curr;

	n_cards = read_bin_int(fp);

	// loading like this preserves cards order (pushing back new cards)
	for (int i = 0; i < n_cards; i++) {
		if (head == NULL)
			curr = head = load_card(fp);
		else
			curr = curr->next = load_card(fp);
	}

	return head;
}

giocatoreT *load_player(FILE *fp) {
	giocatoreT* player;

	player = (giocatoreT*)calloc_checked(ONE_ELEMENT, sizeof(giocatoreT));
	if (fread(player, sizeof(giocatoreT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_read_failed();

	// load player cards now
	player->carte = load_cards(fp);
	player->aula = load_cards(fp);
	player->bonus_malus = load_cards(fp);
	return player;
}

game_contextT* load_game(const char *save_path) {
	FILE *fp;
	game_contextT *game_ctx;
	giocatoreT *curr_player;

	fp = fopen(save_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Opening save file (%s) failed!\n", save_path);
		exit(EXIT_FAILURE);
	}

	game_ctx = (game_contextT*)calloc_checked(ONE_ELEMENT, sizeof(game_contextT));

	init_logging(game_ctx);
	fprintf(game_ctx->log_file, "Caricamento partita da '%s'...\n", save_path);

	game_ctx->n_players = read_bin_int(fp);

	for (int i = 0; i < game_ctx->n_players; i++) {
		if (game_ctx->curr_player == NULL)
			curr_player = game_ctx->curr_player = load_player(fp);
		else
			curr_player = curr_player->next = load_player(fp);
	}
	curr_player->next = game_ctx->curr_player; // make the linked list circular linking tail to head

	game_ctx->mazzo_pesca = load_cards(fp);
	game_ctx->mazzo_scarti = load_cards(fp);
	game_ctx->aula_studio = load_cards(fp);

	// additional info stored in save file: round number. if not present set it to 1
	if (fread(&game_ctx->round_num, sizeof(int), ONE_ELEMENT, fp) != ONE_ELEMENT)
		game_ctx->round_num = 1;

	fclose(fp);

	return game_ctx;
}

void dump_effect(FILE *fp, effettoT *effect) {
	if (fwrite(effect, sizeof(effettoT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
}

void dump_effects(FILE *fp, cartaT *card) {
	for (int i = 0;  i < card->n_effetti; i++)
		dump_effect(fp, &card->effetti[i]);
}

void dump_card(FILE *fp, cartaT *card) {
	if (fwrite(card, sizeof(cartaT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
	dump_effects(fp, card);
}

void dump_cards(FILE *fp, cartaT *head) {
	write_bin_int(fp, count_cards(head));
	for (; head != NULL; head = head->next)
		dump_card(fp, head);
}

void dump_player(FILE *fp, giocatoreT *player) {
	if (fwrite(player, sizeof(giocatoreT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
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

	write_bin_int(fp, game_ctx->n_players);
	for (int i = 0; i < game_ctx->n_players; i++, game_ctx->curr_player = game_ctx->curr_player->next)
		dump_player(fp, game_ctx->curr_player);
	dump_cards(fp, game_ctx->mazzo_pesca); // save mazzo pesca
	dump_cards(fp, game_ctx->mazzo_scarti); // save mazzo scarti
	dump_cards(fp, game_ctx->aula_studio); // save aula studio

	// additional info stored in save file: round number. if not present set it to 1
	write_bin_int(fp, game_ctx->round_num);

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
	if (fscanf(fp, "%d", amount) != ONE_ELEMENT) {
		*amount = 0;
		return NULL;
	}
	
	cartaT *card = (cartaT*)calloc_checked(ONE_ELEMENT, sizeof(cartaT));

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

FILE *open_log_append() {
	FILE *fp = fopen(FILE_LOG, "a");
	if (fp == NULL) {
		fprintf(stderr, "Opening logs file (%s) failed!\n", FILE_LOG);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/**
 * @brief reads one integer from a file stream and ensures correct reading
 * 
 * @param fp file stream
 * @return int read integer
 */
int read_int(FILE *fp) {
	int val;
	if (fscanf(fp, " %d", &val) != ONE_ELEMENT)
		file_read_failed();
	return val;
}

/**
 * @brief writes one integer to a file stream in binary form and ensures successful writing
 * 
 * @param fp file stream
 * @param val integer to write
 */
void write_bin_int(FILE *fp, int val) {
	if (fwrite(&val, sizeof(int), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
}

/**
 * @brief reads one integer from a file stream in binary form and ensures successful reading
 * 
 * @param fp file stream
 * @return int integer read
 */
int read_bin_int(FILE *fp) {
	int val;
	if (fread(&val, sizeof(int), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_read_failed();
	return val;
}
