#ifndef ENUMS_H
#define ENUMS_H

#include "types.h"

enum TipoCarta {
	ALL,
	STUDENTE,
	MATRICOLA,
	STUDENTE_SEMPLICE,
	LAUREANDO,
	BONUS,
	MALUS,
	MAGIA,
	ISTANTANEA
};

enum Azione {
	GIOCA,
	SCARTA,
	ELIMINA,
	RUBA,
	PESCA,
	PRENDI,
	BLOCCA,
	SCAMBIA,
	MOSTRA,
	IMPEDIRE,
	INGEGNERE
};

enum Quando {
	SUBITO,
	INIZIO,
	FINE,
	MAI,
	SEMPRE
};

enum TargetGiocatori {
	IO,
	TU,
	VOI,
	TUTTI
};

const char *quandoT_str(quandoT quando);
const char *target_giocatoriT_str(target_giocatoriT target);
const char *tipo_cartaT_str(tipo_cartaT tipo);
const char *tipo_cartaT_color(tipo_cartaT tipo);
const char *azioneT_str(azioneT azione);
const char *azioneT_verb_str(azioneT azione);

#endif