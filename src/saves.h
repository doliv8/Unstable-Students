#ifndef SAVES_H
#define SAVES_H

#include <stdbool.h>
#include "types.h"

bool valid_save_name(const char *save_name);
void cache_save_name(char *save_path);
char *get_save_path(const char *save_name);
char *ask_save_name(bool new);
char *pick_save(void);

#endif // SAVES_H