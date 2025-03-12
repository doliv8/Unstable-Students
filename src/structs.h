#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "types.h"
#include "enums.h"

// basic game structs
struct Giocatore {
	char name[GIOCATORE_NAME_LEN+1];
	cartaT *carte;
	cartaT *aula;
	cartaT *bonus_malus;
	giocatoreT *next;
};

struct Carta {
	char name[CARTA_NAME_LEN+1];
	char description[CARTA_DESCRIPTION_LEN+1];
	tipo_cartaT tipo;
	int n_effetti;
	effettoT *effetti;
	quandoT quando;
	bool opzionale;
	cartaT *next;
};

struct Effetto {
	azioneT azione;
	target_giocatoriT target_giocatori;
	tipo_cartaT target_carta;
};
// end basic game structs

struct GameContext {
	giocatoreT *curr_player;
	cartaT *mazzo_pesca, *mazzo_scarti, *aula_studio;
	int n_players, round_num;
	bool game_running;
	FILE *log_file;
	char *save_path;
};

struct MultiLineText {
	int n_lines;
	const char **lines;
	int *lengths;
};

struct WrappedText {
	char *text;
	multiline_textT multiline;
};

#endif // STRUCTS_H