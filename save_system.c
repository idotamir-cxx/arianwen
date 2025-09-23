#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "game_types.h"
#include "save_system.h"
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
    char filename[512];  // Increased buffer size
    snprintf(filename, sizeof(filename), "%s/%s.sav", SAVE_DIR, save_name);
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }
    
    fprintf(file, "%d\n", current_node);
    fclose(file);
    return 0;
}

int load_game(const char *save_name) {
    char filename[512];  // Increased buffer size
    snprintf(filename, sizeof(filename), "%s/%s.sav", SAVE_DIR, save_name);
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    int node_id;
    if (fscanf(file, "%d", &node_id) != 1) {
        fclose(file);
        return -1;
    }
    
    fclose(file);
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
    char filename[512];  // Increased buffer size
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
    for (int i = 0; i < num_saves; i++) {
        printf("%d) %s\n", i + 1, saves[i].display_name);
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
    
    printf("Loaded save: %s\n", saves[choice - 1].display_name);
    return node_id;
}
