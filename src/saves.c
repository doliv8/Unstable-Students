#define _GNU_SOURCE

#include "saves.h"
#include "constants.h"
#include "utils.h"
#include "format.h"
#include "string.h"
#include "files.h"

bool valid_save_name(const char *save_name) {
	int save_name_len = strnlen(save_name, SAVE_NAME_LEN);
	bool valid = save_name_len > 0;

	for (int i = 0; i < save_name_len && valid; i++) {
		if (save_name[i] == '.' || save_name[i] == '/' || save_name[i] == '\\')
			valid = false;
	}
	return valid;
}

void cache_save_name(char *save_path) {
	freeable_multiline_textT saves_cache;
	char *save_name;
	bool already_cached = false;

	init_multiline(&saves_cache);
	load_saves_cache(&saves_cache);

	save_name = strdup_checked(&save_path[strlen(SAVES_DIRECTORY)]);
	save_name[strnlen(save_name, SAVE_NAME_LEN + strlen(SAVE_PATH_EXTENSION)) - strlen(SAVE_PATH_EXTENSION)] = '\0';

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

char *get_save_path(const char *save_name) {
	char *save_path;
	asprintf_s(&save_path, SAVES_DIRECTORY "%s" SAVE_PATH_EXTENSION, save_name);
	return save_path;
}

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
char *pick_save() {
	freeable_multiline_textT saves_cache;
	int choice_idx;
	char *save_name;
	bool picked = false;

	init_multiline(&saves_cache);
	load_saves_cache(&saves_cache);

	if (saves_cache.n_lines > 0) {
		for (int i = 0; i < saves_cache.n_lines; i++)
			printf(" [%d] %s\n", i+1, saves_cache.lines[i]);

		puts("Se vuoi caricare un salvataggio non presente nella seguente lista inseriscilo nella cartella '" SAVES_DIRECTORY "'"
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