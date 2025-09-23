#ifndef CHARACTER_SYSTEM_H
#define CHARACTER_SYSTEM_H

#include "game_types.h"

// Character management functions
int create_new_character();
int load_character(Character *character, const char *filename);
int save_character(const Character *character, const char *filename);
void display_character_status();

#endif
