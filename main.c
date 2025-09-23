#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_types.h"
#include "file_loader.h"
#include "save_system.h"
#include "character_system.h"
#include "utils.h"

// Global variables definition
DialogEntry *dialogs = NULL;
TreeNode *tree_nodes = NULL;
Character current_character;
int num_dialogs = 0;
int num_nodes = 0;

// Function prototypes
void play_game(int start_node);
void cleanup();

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <tree_file> <dialog_file>\n", argv[0]);
        return 1;
    }
    
    printf("Loading adventure game...\n\n");
    
    // Load files
    if (load_tree_file(argv[1]) != 0) {
        fprintf(stderr, "Error loading tree file: %s\n", argv[1]);
        cleanup();
        return 1;
    }
    
    if (load_dialog_file(argv[2]) != 0) {
        fprintf(stderr, "Error loading dialog file: %s\n", argv[2]);
        cleanup();
        return 1;
    }
    
    printf("Game loaded successfully!\n\n");
    
    // Create saves directory if it doesn't exist
    create_save_directory();
    
    // Show main menu
    int menu_choice = show_main_menu();
    int start_node = 1;
    
    if (menu_choice == 1) {
        // New game - create character first
        if (create_new_character() != 0) {
            printf("Character creation failed. Exiting.\n");
            cleanup();
            return 1;
        }
    } else if (menu_choice == 2) {
        // Load game
        start_node = show_load_menu();
        if (start_node == -1) {
            printf("No save files found or load cancelled.\n");
            printf("Starting new game...\n\n");
            if (create_new_character() != 0) {
                printf("Character creation failed. Exiting.\n");
                cleanup();
                return 1;
            }
            start_node = 1;
        }
    }
    
    printf("Press Enter to begin...");
    getchar();
    
    // Start the game
    play_game(start_node);
    
    cleanup();
    return 0;
}

void play_game(int start_node) {
    int current_node = start_node;
    int first_screen = 1;  // Don't clear on first display
    
    while (1) {
        TreeNode *node = find_node(current_node);
        if (!node) {
            clear_screen();
            printf("Error: Invalid node %d\n", current_node);
            break;
        }
        
        // Clear screen before displaying new content (except first time)
        if (!first_screen) {
            clear_screen();
        }
        first_screen = 0;
        
        // Display character status
        display_character_status();
        
        // Display dialog for current node
        DialogEntry *dialog = find_dialog(current_node);
        if (dialog) {
            printf("%s\n\n", dialog->text);
        } else {
            printf("Node %d: [No dialog text found]\n\n", current_node);
        }
        
        // Check if this is an ending (no choices)
        if (node->num_choices == 0) {
            printf("=== THE END ===\n");
            printf("\nPress Enter to exit...");
            // Clear any remaining input, then wait for Enter
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            getchar();
            break;
        }
        
        // Display choices
        printf("What do you choose?\n");
        for (int i = 0; i < node->num_choices; i++) {
            printf("%d) %s\n", i + 1, node->choices[i].choice_text);
        }
        
        // Add save and exit options
        int save_option = node->num_choices + 1;
        int exit_option = node->num_choices + 2;
        
        printf("%d) Save Game\n", save_option);
        printf("%d) Exit Game\n", exit_option);
        
        // Get user input
        printf("\nEnter your choice (1-%d): ", exit_option);
        int choice;
        if (scanf("%d", &choice) != 1 || choice < 1 || choice > exit_option) {
            printf("Invalid choice. Please try again.\n");
            printf("Press Enter to continue...");
            
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            getchar();  // Wait for Enter
            continue;
        }
        
        // Clear input buffer after successful input
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        // Handle special choices
        if (choice == save_option) {
            // Save game
            int save_result = show_save_menu(current_node);
            if (save_result == 0) {
                printf("Game saved successfully!\n");
                printf("Press Enter to continue...");
                getchar();
            } else if (save_result == -2) {
                // User cancelled save
                continue;
            } else {
                printf("Failed to save game.\n");
                printf("Press Enter to continue...");
                getchar();
            }
            continue;
        } else if (choice == exit_option) {
            // Exit game
            printf("Are you sure you want to exit? (y/N): ");
            char confirm;
            scanf(" %c", &confirm);
            
            // Clear input buffer
            while ((c = getchar()) != '\n' && c != EOF);
            
            if (confirm == 'y' || confirm == 'Y') {
                printf("Thanks for playing!\n");
                break;
            }
            continue;
        }
        
        // Move to next node (regular choice)
        current_node = node->choices[choice - 1].to_id;
    }
}

void cleanup() {
    if (dialogs) {
        free(dialogs);
        dialogs = NULL;
    }
    if (tree_nodes) {
        free(tree_nodes);
        tree_nodes = NULL;
    }
}
