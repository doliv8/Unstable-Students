#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "card.h"
#include "utils.h"
#include "logging.h"
#include "format.h"
#include "saves.h"

/**
 * @brief call this when an error while reading from a file occurs
 * 
 */
void file_read_failed(void) {
	fputs("Error occurred while reading from a file stream!\n", stderr);
	exit(EXIT_FAILURE);
}

/**
 * @brief call this when an error while writing to a file occurs
 * 
 */
void file_write_failed(void) {
	fputs("Error occurred while writing to a file stream!\n", stderr);
	exit(EXIT_FAILURE);
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

/**
 * @brief reads effects from file
 * 
 * @param fp file stream
 * @param amount count of effects to read
 * @return effettoT* pointer to heap-allocated array of loaded effects
 */
effettoT *load_effects(FILE *fp, size_t amount) {
	effettoT *effects = NULL;

	if (amount > 0) {
		effects = (effettoT*)malloc_checked(amount*sizeof(effettoT));
		if (fread(effects, sizeof(effettoT), amount, fp) != amount)
			file_read_failed();
	}
	return effects;
}

/**
 * @brief reads card from file
 * 
 * @param fp file stream
 * @return cartaT* pointer to loaded card
 */
cartaT *load_card(FILE *fp) {
	cartaT *card;

	card = (cartaT*)malloc_checked(sizeof(cartaT));

	if (fread(card, sizeof(cartaT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_read_failed();

	card->effetti = load_effects(fp, card->n_effetti);
	return card;
}

/**
 * @brief reads cards from file
 * 
 * @param fp file stream
 * @return cartaT* pointer to head of loaded cards linked list
 */
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

/**
 * @brief reads player from file
 * 
 * @param fp file stream
 * @return giocatoreT* pointer to loaded player
 */
giocatoreT *load_player(FILE *fp) {
	giocatoreT *player;

	player = (giocatoreT*)calloc_checked(ONE_ELEMENT, sizeof(giocatoreT));
	if (fread(player, sizeof(giocatoreT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_read_failed();

	// load player cards now
	player->carte = load_cards(fp);
	player->aula = load_cards(fp);
	player->bonus_malus = load_cards(fp);
	return player;
}

/**
 * @brief loads saved game from a given save name
 * 
 * @param save_name name of the save file wanting to load (without extension) located in SAVES_DIRECTORY
 * @return game_contextT* newly created game context or NULL if given save name couldn't be loaded
 */
game_contextT *load_game(const char *save_name) {
	FILE *fp;
	game_contextT *game_ctx;
	giocatoreT *curr_player = NULL;

	if (!valid_save_name(save_name)) {
		printf("Nome salvataggio invalido (%s)!\n", save_name);
		printf("Devi solo specificare il nome (senza estensione) del file di salvataggio presente nella cartella '%s'!\n", SAVES_DIRECTORY);
		return NULL;
	}

	game_ctx = (game_contextT*)calloc_checked(ONE_ELEMENT, sizeof(game_contextT));

	game_ctx->save_path = get_save_path(save_name);

	fp = fopen(game_ctx->save_path, "rb"); // open binary file for reading
	if (fp == NULL) {
		printf("Impossibile aprire il salvataggio indicato (%s)!\n", game_ctx->save_path);
		printf("Assicurati che il file di salvataggio sia in '%s'!\n", SAVES_DIRECTORY);
		// free up allocated memory after failure
		free_wrap(game_ctx->save_path);
		free_wrap(game_ctx);
		return NULL;
	}

	// save save-name in cache after successfully opening it
	cache_save_name(game_ctx->save_path);

	init_logging(game_ctx);
	fprintf(game_ctx->log_file, "Caricamento partita da '%s'...\n", game_ctx->save_path);

	game_ctx->n_players = read_bin_int(fp);

	for (int i = 0; i < game_ctx->n_players; i++) {
		if (game_ctx->curr_player == NULL && curr_player == NULL)
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

/**
 * @brief writes effect to file
 * 
 * @param fp file stream
 * @param effect pointer to effect to dump
 */
void dump_effect(FILE *fp, effettoT *effect) {
	if (fwrite(effect, sizeof(effettoT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
}

/**
 * @brief writes effects to file
 * 
 * @param fp file stream
 * @param card pointer to card whose effects must be dumped
 */
void dump_effects(FILE *fp, cartaT *card) {
	for (int i = 0;  i < card->n_effetti; i++)
		dump_effect(fp, &card->effetti[i]);
}

/**
 * @brief writes card to file
 * 
 * @param fp file stream
 * @param card pointer to card to dump
 */
void dump_card(FILE *fp, cartaT *card) {
	if (fwrite(card, sizeof(cartaT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
	dump_effects(fp, card);
}

/**
 * @brief writes cards to file
 * 
 * @param fp file stream
 * @param head head of cards list to dump
 */
void dump_cards(FILE *fp, cartaT *head) {
	write_bin_int(fp, count_cards(head));
	for (; head != NULL; head = head->next)
		dump_card(fp, head);
}

/**
 * @brief writes to player to file
 * 
 * @param fp file stream
 * @param player pointer to player to dump
 */
void dump_player(FILE *fp, giocatoreT *player) {
	if (fwrite(player, sizeof(giocatoreT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
	dump_cards(fp, player->carte); // save mano
	dump_cards(fp, player->aula); // save aula
	dump_cards(fp, player->bonus_malus); // save bonus/malus
}

/**
 * @brief saves the current game state into the save path
 * 
 * @param game_ctx current game state
 */
void save_game(game_contextT *game_ctx) {
	FILE *fp = fopen(game_ctx->save_path, "wb"); // open binary file for writing
	if (fp == NULL) {
		fprintf(stderr, "Opening save file (%s) failed!\n", game_ctx->save_path);
		exit(EXIT_FAILURE);
	}

	log_s(game_ctx, "Salvataggio su '%s' in corso...", game_ctx->save_path);

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

/**
 * @brief parses effects array from the given file stream
 * 
 * @param fp file stream
 * @param n_effects number of parsed effects
 * @return effettoT* pointer to heap-allocated array containing the parsed effects
 */
effettoT *read_effetti(FILE *fp, int *n_effects) {
	int amount = read_int(fp);
	effettoT *effects = NULL;
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

/**
 * @brief reads a card from fp returning its amount aswell. chains the read card onto the current linked list (trough tail_next pointer).
 * 
 * @param fp file stream
 * @param tail_next pointer to the current tail's next pointer
 * @param amount out parameter containing the number of cards like this present (count of duplicates+1), 0 if no more cards are readable
 * @return cartaT* the new linked list tail
 */
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
	// add additional copies
	for (int i = 1; i < *amount; i++) // start from 1 as the first one has already been added to the linked list
		card = card->next = duplicate_carta(card);

	// set last allocated card's next ptr to NULL and return the new linked list tail
	card->next = NULL;
	return card;
}

/**
 * @brief loads all the cards from the FILE_MAZZO file
 * 
 * @param n_cards out parameter containing number of loaded cards
 * @return cartaT* head of the mazzo cards linked list
 */
cartaT *load_mazzo(int *n_cards) {
	FILE *fp = fopen(FILE_MAZZO, "r");
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

/**
 * @brief opens log file for appending (creating it if it doesn't exist)
 * 
 * @return FILE* log file stream
 */
FILE *open_log_append(void) {
	FILE *fp = fopen(FILE_LOG, "a");
	if (fp == NULL) {
		fprintf(stderr, "Opening logs file (%s) failed!\n", FILE_LOG);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/**
 * @brief loads the saves name cache from FILE_SAVES_CACHE file (creates it if it doesn't exist) into saves multiline
 * 
 * @param saves pointer to already initialized freeable_multiline_textT which will contain one save name (without SAVE_PATH_EXTENSION) per line
 */
void load_saves_cache(freeable_multiline_textT *saves) {
	char save_name[SAVE_NAME_LEN+1];
	FILE *fp = fopen(SAVES_DIRECTORY FILE_SAVES_CACHE, "r");
	if (fp == NULL) {
		// either the cache file doesn't exist or the whole SAVES_DIRECTORY doesn't exist.
		// try creating the FILE_SAVES_CACHE file to check if directory exists.
		fp = fopen(SAVES_DIRECTORY FILE_SAVES_CACHE, "w");
		if (fp == NULL) {
			fprintf(stderr, "Opening saves cache file (%s) failed!\nAssicurati che la cartella '%s' esista e se non esiste creala!\n",
				SAVES_DIRECTORY FILE_SAVES_CACHE, SAVES_DIRECTORY);
			exit(EXIT_FAILURE);
		}
	} else {
		while (fscanf(fp, " %" TO_STRING(SAVE_NAME_LEN) "[^\n]", save_name) > 0)
			multiline_addline(saves, strdup_checked(save_name));
	}
	fclose(fp);
}

/**
 * @brief saves the given saves name into the FILE_SAVES_CACHE file
 * 
 * @param saves pointer to freeable_multiline_textT containing one save name (without SAVE_PATH_EXTENSION) per line
 */
void save_saves_cache(freeable_multiline_textT *saves) {
	FILE *fp = fopen(SAVES_DIRECTORY FILE_SAVES_CACHE, "w");
	if (fp == NULL) {
		// the SAVES_DIRECTORY directory doesn't exist.
		fprintf(stderr, "Opening saves cache file (%s) failed!\nAssicurati che la cartella '%s' esista e se non esiste creala!\n",
			SAVES_DIRECTORY FILE_SAVES_CACHE, SAVES_DIRECTORY);
		exit(EXIT_FAILURE);
	} else {
		for (int i = 0; i < saves->n_lines; i++)
			fprintf(fp, "%s\n", saves->lines[i]);
	}
	fclose(fp);
}

/**
 * @brief opens stats file for reading (creating it if it doesn't exist)
 * 
 * @return FILE* stats file stream
 */
FILE *open_stats_read(void) {
	FILE *fp = fopen(FILE_STATS, "rb"); // open binary file for reading
	if (fp == NULL) { // stats file doesn't exist
		fp = fopen(FILE_STATS, "wb"); // open binary file for writing
		if (fp != NULL) { // file created successfully
			fclose(fp);
			fp = fopen(FILE_STATS, "rb"); // its a binary file
		} else {
			fprintf(stderr, "Creating stats file (%s) failed!\n", FILE_STATS);
			exit(EXIT_FAILURE);
		}
	}
	if (fp == NULL) {
		fprintf(stderr, "Opening stats file (%s) failed!\n", FILE_STATS);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/**
 * @brief opens stats file for reading and writing
 * 
 * @return FILE* stats file stream
 */
FILE *open_stats_read_write(void) {
	FILE *fp = fopen(FILE_STATS, "rb+"); // open binary file for reading and writing
	if (fp == NULL) {
		fprintf(stderr, "Opening stats file (%s) failed!\nIl file dovrebbe esistere dato che il gioco e' gia' avviato!", FILE_STATS);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/**
 * @brief reads player stats into the given stats pointer from the file stream
 * 
 * @param fp file stream (allowing reading)
 * @param stats pointer to player stats struct to be read into (out parameter)
 * @return true if successfully read one player stats
 * @return false if couldn't read one player stats
 */
bool read_player_stats(FILE *fp, player_statsT *stats) {
	if (fread(stats, sizeof(player_statsT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		return false;
	return true;
}

/**
 * @brief writes the given player stats into the file stream
 * 
 * @param fp file stream (allowing writing)
 * @param stats pointer to player stats struct to be written into the file
 */
void write_player_stats(FILE *fp, player_statsT *stats) {
	if (fwrite(stats, sizeof(player_statsT), ONE_ELEMENT, fp) != ONE_ELEMENT)
		file_write_failed();
}