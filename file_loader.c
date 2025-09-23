#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_types.h"
#include "file_loader.h"

int is_valid_ability(const char *ability_name) {
    const char* valid_abilities[] = {"Strength", "Intelligence", "Wisdom", "Dexterity", "Constitution", "Charisma"};
    int num_abilities = 6;

    for (int i = 0; i < num_abilities; i++) {
        if (strcmp(ability_name, valid_abilities[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int parse_choice_line(const char *line, Choice *choice, int from_id) {
    // Initialize choice
    choice->from_id = from_id;
    choice->choice_type = CHOICE_REGULAR;
    memset(choice->ability_name, 0, sizeof(choice->ability_name));

    // Find the arrow
    char *arrow = strstr(line, "->");
    if (!arrow) return -1;

    // Extract choice text (skip leading whitespace)
    char *start = (char*)line;
    while (*start == ' ' || *start == '\t') start++;

    int text_len = arrow - start;
    strncpy(choice->choice_text, start, text_len);
    choice->choice_text[text_len] = '\0';

    // Remove trailing whitespace from choice text
    while (text_len > 0 && (choice->choice_text[text_len-1] == ' ' || choice->choice_text[text_len-1] == '\t')) {
        choice->choice_text[--text_len] = '\0';
    }

    // Check if this is an ability check by looking for parentheses
    char *open_paren = strchr(choice->choice_text, '(');
    char *close_paren = strchr(choice->choice_text, ')');

    if (open_paren && close_paren && close_paren > open_paren) {
        // Extract ability name from parentheses
        char *check_start = open_paren + 1;
        char *check_word = strstr(check_start, " check");

        if (check_word && check_word < close_paren) {
            // Extract ability name
            int ability_len = check_word - check_start;
            char temp_ability[50];
            strncpy(temp_ability, check_start, ability_len);
            temp_ability[ability_len] = '\0';

            // Validate ability name
            if (is_valid_ability(temp_ability)) {
                choice->choice_type = CHOICE_ABILITY_CHECK;
                strcpy(choice->ability_name, temp_ability);

                // Parse target nodes (success,failure)
                char *targets = arrow + 2;
                while (*targets == ' ') targets++; // Skip whitespace

                char *comma = strchr(targets, ',');
                if (comma) {
                    *comma = '\0';
                    choice->target.check_nodes.success_node = atoi(targets);
                    choice->target.check_nodes.failure_node = atoi(comma + 1);
                    return 0;
                } else {
                    // Invalid format for ability check
                    return -1;
                }
            }
        }
    }

    // Regular choice - single target
    choice->target.to_id = atoi(arrow + 2);
    return 0;
}

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

            Choice *choice = &current_node->choices[current_node->num_choices];

            if (parse_choice_line(line, choice, current_node->node_id) == 0) {
                current_node->num_choices++;
            }
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