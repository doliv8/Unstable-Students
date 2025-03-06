#include "types.h"
#include "enums.h"
#include "constants.h"

const char *quandoT_str(quandoT quando) {
	static const char *mapping[] = {
		[SUBITO] = "Subito",
		[INIZIO] = "Inizio",
		[FINE] = "Fine",
		[MAI] = "Mai",
		[SEMPRE] = "Sempre"
	};
	return mapping[quando];
}

const char *target_giocatoriT_str(target_giocatoriT target) {
	static const char *mapping[] = {
		[IO] = "Io",
		[TU] = "Tu",
		[VOI] = "Voi",
		[TUTTI] = "Tutti"
	};
	return mapping[target];
}

const char *tipo_cartaT_str(tipo_cartaT tipo) {
	static const char *mapping[] = {
		[ALL] = "Qualsiasi",
		[STUDENTE] = "Studente",
		[MATRICOLA] = "Matricola",
		[STUDENTE_SEMPLICE] = "Studente semplice",
		[LAUREANDO] = "Laureando",
		[BONUS] = "Bonus",
		[MALUS] = "Malus",
		[MAGIA] = "Magia",
		[ISTANTANEA] = "Istantanea"
	};
	return mapping[tipo];
}

const char *tipo_cartaT_color(tipo_cartaT tipo) {
	static const char *mapping[] = {
		[ALL] = ANSI_RESET,
		[STUDENTE] = ANSI_RESET,
		[MATRICOLA] = ANSI_CYAN,
		[STUDENTE_SEMPLICE] = ANSI_CYAN,
		[LAUREANDO] = ANSI_YELLOW,
		[BONUS] = ANSI_GREEN,
		[MALUS] = ANSI_RED,
		[MAGIA] = ANSI_MAGENTA,
		[ISTANTANEA] = ANSI_BLUE
	};
	return mapping[tipo];
}

const char *azioneT_str(azioneT azione) {
	static const char *mapping[] = {
		[GIOCA] = "Gioca",
		[SCARTA] = "Scarta",
		[ELIMINA] = "Elimina",
		[RUBA] = "Ruba",
		[PESCA] = "Pesca",
		[PRENDI] = "Prendi",
		[BLOCCA] = "Blocca",
		[SCAMBIA] = "Scambia",
		[MOSTRA] = "Mostra",
		[IMPEDIRE] = "Impedire",
		[INGEGNERE] = "Ingegnere"
	};
	return mapping[azione];
}