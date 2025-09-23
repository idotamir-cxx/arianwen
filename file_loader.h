#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include "game_types.h"

// File loading functions
int load_dialog_file(const char *filename);
int load_tree_file(const char *filename);

// Search functions
TreeNode* find_node(int node_id);
DialogEntry* find_dialog(int dialog_id);

#endif
