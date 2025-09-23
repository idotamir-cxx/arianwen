#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_types.h"
#include "file_loader.h"

int load_dialog_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    char line[MAX_LINE_LENGTH];
    int capacity = 10;
    
    dialogs = malloc(capacity * sizeof(DialogEntry));
    if (!dialogs) {
        fclose(file);
        return -1;
    }
    
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') continue;
        
        // Resize array if needed
        if (num_dialogs >= capacity) {
            capacity *= 2;
            DialogEntry *temp = realloc(dialogs, capacity * sizeof(DialogEntry));
            if (!temp) {
                fclose(file);
                return -1;
            }
            dialogs = temp;
        }
        
        // Parse: ID:Dialog text
        char *colon = strchr(line, ':');
        if (!colon) continue;
        
        *colon = '\0';
        dialogs[num_dialogs].id = atoi(line);
        
        // Copy text, removing newline
        strncpy(dialogs[num_dialogs].text, colon + 1, MAX_TEXT_LENGTH - 1);
        dialogs[num_dialogs].text[MAX_TEXT_LENGTH - 1] = '\0';
        
        // Remove trailing newline
        char *newline = strchr(dialogs[num_dialogs].text, '\n');
        if (newline) *newline = '\0';
        
        num_dialogs++;
    }
    
    fclose(file);
    return 0;
}

int load_tree_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    char line[MAX_LINE_LENGTH];
    int capacity = 10;
    
    tree_nodes = malloc(capacity * sizeof(TreeNode));
    if (!tree_nodes) {
        fclose(file);
        return -1;
    }
    
    TreeNode *current_node = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') continue;
        
        // Remove trailing newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (line[0] != ' ' && line[0] != '\t') {
            // New node definition
            if (num_nodes >= capacity) {
                capacity *= 2;
                TreeNode *temp = realloc(tree_nodes, capacity * sizeof(TreeNode));
                if (!temp) {
                    fclose(file);
                    return -1;
                }
                tree_nodes = temp;
            }
            
            current_node = &tree_nodes[num_nodes];
            current_node->node_id = atoi(line);
            current_node->num_choices = 0;
            num_nodes++;
        } else {
            // Choice definition (indented line)
            if (!current_node || current_node->num_choices >= MAX_CHOICES) {
                continue;
            }
            
            // Parse: "  choice_text -> target_id"
            char *arrow = strstr(line, "->");
            if (!arrow) continue;
            
            Choice *choice = &current_node->choices[current_node->num_choices];
            choice->from_id = current_node->node_id;
            choice->to_id = atoi(arrow + 2);
            
            // Extract choice text (skip leading whitespace)
            char *start = line;
            while (*start == ' ' || *start == '\t') start++;
            
            int len = arrow - start;
            strncpy(choice->choice_text, start, len);
            choice->choice_text[len] = '\0';
            
            // Remove trailing whitespace
            while (len > 0 && (choice->choice_text[len-1] == ' ' || choice->choice_text[len-1] == '\t')) {
                choice->choice_text[--len] = '\0';
            }
            
            current_node->num_choices++;
        }
    }
    
    fclose(file);
    return 0;
}

TreeNode* find_node(int node_id) {
    for (int i = 0; i < num_nodes; i++) {
        if (tree_nodes[i].node_id == node_id) {
            return &tree_nodes[i];
        }
    }
    return NULL;
}

DialogEntry* find_dialog(int dialog_id) {
    for (int i = 0; i < num_dialogs; i++) {
        if (dialogs[i].id == dialog_id) {
            return &dialogs[i];
        }
    }
    return NULL;
}
