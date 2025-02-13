/*
* only used for debug builds (compiling with -DDEBUG parameter)
*/
#ifdef DEBUG
#include "types.h"
#include "structs.h"
#include "graphics.h"
#include "gameplay.h"

void dump_full_game(game_contextT *game_ctx) {
	show_card_group(game_ctx->mazzo_pesca, "Mazzo Pesca", ANSI_RED "%s" ANSI_RESET);
	show_card_group(game_ctx->mazzo_scarti, "Mazzo Scarti", ANSI_RED "%s" ANSI_RESET);
	show_card_group(game_ctx->aula_studio, "Aula Studio", ANSI_RED "%s" ANSI_RESET);
	
	show_round(game_ctx);

	for (int i = 0; i < game_ctx->n_players; i++, game_ctx->curr_player = game_ctx->curr_player->next)
		view_own(game_ctx);
}
#endif // DEBUG