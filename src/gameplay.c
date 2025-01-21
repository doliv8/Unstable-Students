#include "gameplay.h"

/*
void view_others(game_contextT* game_ctx) {
	// TODO: implement viewing others' cards
	puts("Scegli il giocatore del quale vuoi vedere lo stato:");
	giocatoreT* player = game_ctx->next_player->next;
	for (int i = 1; i < game_ctx->n_players; i++, player = player->next)
		printf(" [TASTO %d] %s\n", i, player->name);
	printf(" [TASTO %d] Tutti i giocatori\n", game_ctx->n_players);
	int chosen_idx = get_int();
	printf("Ecco lo stato di %s:\n", player->name);
	
}*/