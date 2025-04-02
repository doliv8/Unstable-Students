#define _GNU_SOURCE

#include <string.h>
#include "saves.h"
#include "constants.h"
#include "utils.h"
#include "format.h"
#include "files.h"

/**
 * @brief checks if given save name is valid
 * 
 * @param save_name save name to be checked for validity
 * @return true if given save name is valid
 * @return false if given save name isn't valid
 */
bool valid_save_name(const char *save_name) {
	int save_name_len = strlen(save_name);
	bool valid = save_name_len > 0 && save_name_len <= SAVE_NAME_LEN;

	for (int i = 0; i < save_name_len && valid; i++) {
		if (save_name[i] == '.' || save_name[i] == '/' || save_name[i] == '\\')
			valid = false;
	}
	return valid;
}

/**
 * @brief stores given save path's save to saves cache
 * 
 * @param save_path relative path of save to be cached
 */
void cache_save_name(char *save_path) {
	freeable_multiline_textT saves_cache;
	char *save_name;
	bool already_cached = false;

	init_multiline(&saves_cache);
	load_saves_cache(&saves_cache);

	save_name = strdup_checked(&save_path[strlen(SAVES_DIRECTORY)]); // skip SAVES_DIRECTORY substring
	// make substring stripping save file extension
	save_name[strnlen(save_name, SAVE_NAME_LEN + strlen(SAVE_PATH_EXTENSION)) - strlen(SAVE_PATH_EXTENSION)] = '\0';

	// check if save name is already present in cache
	for (int i = 0; i < saves_cache.n_lines && !already_cached; i++)
		if (!strncmp(saves_cache.lines[i], save_name, SAVE_NAME_LEN))
			already_cached = true;

	// if not already present in cache, add it.
	if (!already_cached) {
		multiline_addline(&saves_cache, save_name);
		save_saves_cache(&saves_cache);
	} else
		free_wrap(save_name);

	clear_freeable_multiline(&saves_cache);
}

/**
 * @brief combines save name into relative path of given save
 * 
 * @param save_name save name refering to a save in the saves directory
 * @return char* heap-allocated string containing relative path to the save relative to given save name
 */
char *get_save_path(const char *save_name) {
	char *save_path;
	asprintf_s(&save_path, SAVES_DIRECTORY "%s" SAVE_PATH_EXTENSION, save_name);
	return save_path;
}

/**
 * @brief prompts user to choose a name for the save, to load or create
 * 
 * @param new is the asked name for a new save?
 * @return char* heap-allocated string containing inserted name
 */
char *ask_save_name(bool new) {
	char save_name[SAVE_NAME_LEN+1];

	do {
		if (new)
			printf("Che nome vuoi dare al salvataggio? ");
		else
			printf("Inserisci il nome del salvataggio da caricare: ");
		scanf(" %" TO_STRING(SAVE_NAME_LEN) "[^\n]", save_name);
	} while (!valid_save_name(save_name));

	return strdup_checked(save_name);
}

/**
 * @brief prompts user to choice a saved game to load using saved games cache and manual prompt.
 * 
 * @return char* the name of the chosen save to load (heap-allocated string)
 */
char *pick_save(void) {
	freeable_multiline_textT saves_cache;
	int choice_idx;
	char *save_name;
	bool picked = false;

	init_multiline(&saves_cache);
	load_saves_cache(&saves_cache);

	if (saves_cache.n_lines > 0) {
		puts("Ecco gli ultimi salvataggi:");
		for (int i = 0; i < saves_cache.n_lines; i++)
			printf(" [%d] %s\n", i+1, saves_cache.lines[i]);

		puts("Se vuoi caricare un salvataggio non presente nella seguente lista spostalo nella cartella '" SAVES_DIRECTORY "'"
			" e inseriscine il nome manualmente dopo aver selezionato 'no'. Sara' inserito nella lista per il futuro una volta caricato.");
		printf("Vuoi caricare un salvataggio da questa lista? ");
		if (ask_choice()) {
			do {
				puts("Scegli quale salvataggio vuoi caricare.");
				choice_idx = get_int();
			} while (choice_idx < 1 || choice_idx > saves_cache.n_lines);
			save_name = strdup_checked(saves_cache.lines[choice_idx-1]);
			picked = true;
		}
	}
	if (!picked)
		save_name = ask_save_name(false);

	clear_freeable_multiline(&saves_cache);
	return save_name;
}