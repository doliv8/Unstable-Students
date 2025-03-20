#ifndef EFFECTS_H
#define EFFECTS_H

#include "types.h"

void apply_effect_elimina_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_scarta_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_gioca_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_ruba_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_prendi_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_pesca_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);
void apply_effect_scambia_target(game_contextT *game_ctx, giocatoreT *target, effettoT *effect);

#endif // EFFECTS_H