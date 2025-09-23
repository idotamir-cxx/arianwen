#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "game_types.h"

// Save/Load functions
void create_save_directory();
int save_game(int current_node, const char *save_name);
int load_game(const char *save_name);
int list_save_files(SaveFile saves[], int max_saves);

// Menu functions
int show_save_menu(int current_node);
int show_load_menu();

#endif
