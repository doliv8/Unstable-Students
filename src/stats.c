#include <string.h>
#include "stats.h"
#include "files.h"
#include "utils.h"
#include "card.h"

/**
 * @brief displays stats of all registered players showing maximums for wins, rounds, discarded and played aswell
 * 
 */
void display_full_stats(void) {
	player_statsT stats, max_wins, max_rounds, max_discarded, max_played;
	int count;
	FILE *fp = open_stats_read();

	for (count = 0; read_player_stats(fp, &stats); count++) {
		if (count == 0)
			max_wins = max_rounds = max_discarded = max_played = stats; // initialize maximums to first read stats

		printf(ANSI_CYAN "\nStatistiche di " ANSI_BOLD ANSI_RED PRETTY_USERNAME ANSI_CYAN ":\n\n" ANSI_RESET, stats.name);

		printf("  Totale partite vinte: " ANSI_BOLD "%d" ANSI_RESET "\n", stats.wins);
		printf("  Totale round giocati: " ANSI_BOLD "%d" ANSI_RESET "\n", stats.rounds);
		printf("  Carte scartate in totale: " ANSI_BOLD "%d" ANSI_RESET "\n", stats.rounds);
		printf("  Carte giocate in totale: " ANSI_BOLD "%d" ANSI_RESET "\n\n", stats.played_cards[ALL]);

		for (tipo_cartaT type = STUDENTE; type <= ISTANTANEA; type++) { // iterate over card type enum (ALL excluded)
			printf("  Numero carte " COLORED_CARD_TYPE " giocate: " ANSI_BOLD "%d" ANSI_RESET "\n",
				tipo_cartaT_color(type),
				tipo_cartaT_str(type),
				stats.played_cards[type]
			);
		}
		// keep track of maximums
		if (stats.wins > max_wins.wins)
			max_wins = stats;
		if (stats.rounds > max_rounds.rounds)
			max_rounds = stats;
		if (stats.discarded > max_discarded.discarded)
			max_discarded = stats;
		if (stats.played_cards[ALL] > max_played.played_cards[ALL])
			max_played = stats;
	}

	if (count != 0) {
		printf("\nIl giocatore che ha vinto piu' partite di tutti e': " ANSI_BOLD ANSI_RED PRETTY_USERNAME " (%d partite vinte)!\n",
			max_wins.name,
			max_wins.wins
		);
		printf("Il giocatore che ha giocato piu' round di tutti e': " ANSI_BOLD ANSI_RED PRETTY_USERNAME " (%d round giocati in totale)!\n",
			max_rounds.name,
			max_rounds.rounds
		);
		printf("Il giocatore che ha scartato piu' carte di tutti e': " ANSI_BOLD ANSI_RED PRETTY_USERNAME " (%d carte scartate)!\n",
			max_discarded.name,
			max_discarded.discarded
		);
		printf("Il giocatore che ha giocato piu' carte di tutti e': " ANSI_BOLD ANSI_RED PRETTY_USERNAME " (%d carte giocate in tutto)!\n\n",
			max_played.name,
			max_played.played_cards[ALL]
		);
	} else {
		puts("Non sono presenti giocatori passati di cui visualizzare le statistiche.\n");
	}

	fclose(fp);
}

/**
 * @brief loads statistics from stats file for the given player and returns new empty stats if player doesn't exist in stats file.
 * 
 * @param player player to load stats for
 * @return player_statsT* player statistics loaded from file or newly created (empty)
 */
player_statsT *load_player_stats(giocatoreT *player) {
	player_statsT stats, *new_stats = calloc_checked(ONE_ELEMENT, sizeof(player_statsT));
	bool found = false;
	FILE *fp = open_stats_read();

	while (read_player_stats(fp, &stats) && !found) {
		if (!strncmp(stats.name, player->name, GIOCATORE_NAME_LEN)) {
			*new_stats = stats;
			found = true;
		}
	}
	if (!found)
		strncpy(new_stats->name, player->name, sizeof(new_stats->name));

	fclose(fp);
	return new_stats;
}

/**
 * @brief load statistics for each player playing this game
 * 
 * @param game_ctx current game state
 */
void load_stats(game_contextT *game_ctx) {
	giocatoreT *player;
	player_statsT *curr_stats = NULL;

	// game_ctx->curr_stats serves as the linked-list head
	player = game_ctx->curr_player;
	for (int i = 0; i < game_ctx->n_players; i++, player = player->next) {
		if (game_ctx->curr_stats == NULL && curr_stats == NULL)
			curr_stats = game_ctx->curr_stats = load_player_stats(player); // set linked list head
		else
			curr_stats = curr_stats->next = load_player_stats(player);
	}
	curr_stats->next = game_ctx->curr_stats; // make the linked list circular linking tail to head
}

/**
 * @brief adds a game win to current player stats
 * 
 * @param game_ctx current game state
 */
void stats_add_win(game_contextT *game_ctx) {
	game_ctx->curr_stats->wins++;
}

/**
 * @brief adds a played round current player to stats
 * 
 * @param game_ctx current game state
 */
void stats_add_round(game_contextT *game_ctx) {
	game_ctx->curr_stats->rounds++;
}

/**
 * @brief adds a discarded card current player to stats
 * 
 * @param game_ctx current game state
 */
void stats_add_discarded(game_contextT *game_ctx) {
	game_ctx->curr_stats->discarded++;
}

/**
 * @brief adds a played card to the played card types stats of current player
 * 
 * @param game_ctx current game state
 * @param card played card
 */
void stats_add_played_card(game_contextT *game_ctx, cartaT *card) {
	for (tipo_cartaT type = ALL; type <= ISTANTANEA; type++) { // iterate over card type enum
		if (match_card_type(card, type))
			game_ctx->curr_stats->played_cards[type]++; // use enum as array index, as it is a number
	}
}

/**
 * @brief updates stats for a player in the stats file
 * 
 * @param stats_update updated player stats
 */
void update_stats(player_statsT *stats_update) {
	player_statsT stats;
	bool found = false;
	FILE *fp = open_stats_read_write();

	while (!found && read_player_stats(fp, &stats)) {
		if (!strncmp(stats.name, stats_update->name, GIOCATORE_NAME_LEN)) {
			fseek(fp, -(long)sizeof(player_statsT), SEEK_CUR); // move cursor back to the start of the just read stats, to overwrite them
			found = true;
		}
	}
	// write stats update, overwriting same player's stats if found, appending new stats if not found
	write_player_stats(fp, stats_update); 

	fclose(fp);
}

/**
 * @brief updates stats for each player in the game to the stats file
 * 
 * @param game_ctx current game state
 */
void save_stats(game_contextT *game_ctx) {
	player_statsT *curr_stats = game_ctx->curr_stats;
	for (int i = 0; i < game_ctx->n_players; i++, curr_stats = curr_stats->next)
		update_stats(curr_stats);
}