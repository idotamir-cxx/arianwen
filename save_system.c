#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "game_types.h"
#include "save_system.h"
#include "character_system.h"
#include "utils.h"

void create_save_directory() {
    struct stat st = {0};
    if (stat(SAVE_DIR, &st) == -1) {
#ifdef _WIN32
        mkdir(SAVE_DIR);
#else
        mkdir(SAVE_DIR, 0700);
#endif
    }
}

int save_game(int current_node, const char *save_name) {
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.sav", SAVE_DIR, save_name);

    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }

    // Save game state
    fprintf(file, "NODE:%d\n", current_node);

    // Save character data directly in save file
    fprintf(file, "CHARACTER_NAME:%s\n", current_character.name);
    fprintf(file, "CHARACTER_CLASS:%s\n", current_character.class);
    fprintf(file, "CHARACTER_ALIGNMENT:%s\n", current_character.alignment);
    fprintf(file, "CHARACTER_HP:%d\n", current_character.hit_points);
    fprintf(file, "CHARACTER_MAX_HP:%d\n", current_character.max_hit_points);
    fprintf(file, "CHARACTER_STR:%d\n", current_character.abilities.strength);
    fprintf(file, "CHARACTER_INT:%d\n", current_character.abilities.intelligence);
    fprintf(file, "CHARACTER_WIS:%d\n", current_character.abilities.wisdom);
    fprintf(file, "CHARACTER_DEX:%d\n", current_character.abilities.dexterity);
    fprintf(file, "CHARACTER_CON:%d\n", current_character.abilities.constitution);
    fprintf(file, "CHARACTER_CHA:%d\n", current_character.abilities.charisma);

    fclose(file);
    return 0;
}

int load_game(const char *save_name) {
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.sav", SAVE_DIR, save_name);

    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    char line[256];
    int node_id = -1;
    int found_character_data = 0;

    // Initialize character
    memset(&current_character, 0, sizeof(Character));

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = line;
        char *value = colon + 1;

        if (strcmp(key, "NODE") == 0) {
            node_id = atoi(value);
        } else if (strcmp(key, "CHARACTER_NAME") == 0) {
            strncpy(current_character.name, value, MAX_NAME_LENGTH - 1);
            found_character_data = 1;
        } else if (strcmp(key, "CHARACTER_CLASS") == 0) {
            strncpy(current_character.class, value, MAX_CLASS_LENGTH - 1);
        } else if (strcmp(key, "CHARACTER_ALIGNMENT") == 0) {
            strncpy(current_character.alignment, value, MAX_ALIGNMENT_LENGTH - 1);
        } else if (strcmp(key, "CHARACTER_HP") == 0) {
            current_character.hit_points = atoi(value);
        } else if (strcmp(key, "CHARACTER_MAX_HP") == 0) {
            current_character.max_hit_points = atoi(value);
        } else if (strcmp(key, "CHARACTER_STR") == 0) {
            current_character.abilities.strength = atoi(value);
        } else if (strcmp(key, "CHARACTER_INT") == 0) {
            current_character.abilities.intelligence = atoi(value);
        } else if (strcmp(key, "CHARACTER_WIS") == 0) {
            current_character.abilities.wisdom = atoi(value);
        } else if (strcmp(key, "CHARACTER_DEX") == 0) {
            current_character.abilities.dexterity = atoi(value);
        } else if (strcmp(key, "CHARACTER_CON") == 0) {
            current_character.abilities.constitution = atoi(value);
        } else if (strcmp(key, "CHARACTER_CHA") == 0) {
            current_character.abilities.charisma = atoi(value);
        }
    }

    fclose(file);

    // Check if we have valid data
    if (node_id == -1) {
        return -1;
    }

    // If no character data found, it might be an old save file format
    if (!found_character_data) {
        printf("Warning: Save file contains no character data. You may need to create a new character.\n");
    }

    return node_id;
}

int list_save_files(SaveFile saves[], int max_saves) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(SAVE_DIR);
    if (!dir) {
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && count < max_saves) {
        // Check if it's a .sav file
        int len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".sav") == 0) {
            // Copy filename
            snprintf(saves[count].filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);

            // Create display name (remove .sav extension)
            strncpy(saves[count].display_name, entry->d_name, len - 4);
            saves[count].display_name[len - 4] = '\0';

            count++;
        }
    }

    closedir(dir);
    return count;
}

int show_save_menu(int current_node) {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║           SAVE GAME              ║\n");
    printf("╚══════════════════════════════════╝\n\n");

    printf("Current Character: %s the %s\n\n", current_character.name, current_character.class);

    printf("Enter save name (or 'cancel' to return): ");
    char save_name[MAX_FILENAME_LENGTH];

    if (fgets(save_name, sizeof(save_name), stdin) == NULL) {
        return -1;
    }

    // Remove newline
    char *newline = strchr(save_name, '\n');
    if (newline) *newline = '\0';

    // Check for cancel
    if (strcmp(save_name, "cancel") == 0) {
        return -2;
    }

    // Check if file already exists
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.sav", SAVE_DIR, save_name);

    if (file_exists(filename)) {
        printf("Save file '%s' already exists. Overwrite? (y/N): ", save_name);
        char confirm;
        scanf(" %c", &confirm);

        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (confirm != 'y' && confirm != 'Y') {
            return -2;
        }
    }

    return save_game(current_node, save_name);
}

int show_load_menu() {
    SaveFile saves[MAX_SAVES];
    int num_saves = list_save_files(saves, MAX_SAVES);

    if (num_saves == 0) {
        clear_screen();
        printf("No save files found.\n");
        printf("Press Enter to continue...");
        getchar();
        return -1;
    }

    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║           LOAD GAME              ║\n");
    printf("╚══════════════════════════════════╝\n\n");

    printf("Available save files:\n\n");

    // Preview save files with character info
    for (int i = 0; i < num_saves; i++) {
        char preview_info[256] = "";

        // Try to read character info from save file
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s.sav", SAVE_DIR, saves[i].display_name);

        FILE *file = fopen(full_path, "r");
        if (file) {
            char line[256];
            char char_name[MAX_NAME_LENGTH] = "";
            char char_class[MAX_CLASS_LENGTH] = "";

            while (fgets(line, sizeof(line), file)) {
                char *newline_pos = strchr(line, '\n');
                if (newline_pos) *newline_pos = '\0';

                char *colon = strchr(line, ':');
                if (!colon) continue;

                *colon = '\0';
                char *key = line;
                char *value = colon + 1;

                if (strcmp(key, "CHARACTER_NAME") == 0) {
                    strncpy(char_name, value, MAX_NAME_LENGTH - 1);
                } else if (strcmp(key, "CHARACTER_CLASS") == 0) {
                    strncpy(char_class, value, MAX_CLASS_LENGTH - 1);
                }

                // Stop if we have both pieces of info
                if (strlen(char_name) > 0 && strlen(char_class) > 0) {
                    break;
                }
            }

            if (strlen(char_name) > 0 && strlen(char_class) > 0) {
                snprintf(preview_info, sizeof(preview_info), " - %s the %s", char_name, char_class);
            }

            fclose(file);
        }

        printf("%d) %s%s\n", i + 1, saves[i].display_name, preview_info);
    }

    printf("%d) Cancel\n", num_saves + 1);

    printf("\nEnter your choice (1-%d): ", num_saves + 1);
    int choice;

    while (scanf("%d", &choice) != 1 || choice < 1 || choice > num_saves + 1) {
        printf("Invalid choice. Please enter 1-%d: ", num_saves + 1);
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (choice == num_saves + 1) {
        return -1; // Cancel
    }

    // Load the selected save
    int node_id = load_game(saves[choice - 1].display_name);
    if (node_id == -1) {
        printf("Error loading save file.\n");
        printf("Press Enter to continue...");
        getchar();
        return -1;
    }

    printf("Loaded save: %s", saves[choice - 1].display_name);
    if (strlen(current_character.name) > 0) {
        printf(" - %s the %s", current_character.name, current_character.class);
    }
    printf("\n");

    return node_id;
}