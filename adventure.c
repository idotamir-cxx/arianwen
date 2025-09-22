#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 1024
#define MAX_CHOICES 10
#define MAX_TEXT_LENGTH 2048
#define MAX_FILENAME_LENGTH 256
#define MAX_SAVES 20
#define SAVE_DIR "saves"

typedef struct {
    int id;
    char text[MAX_TEXT_LENGTH];
} DialogEntry;

typedef struct {
    int from_id;
    int to_id;
    char choice_text[MAX_LINE_LENGTH];
} Choice;

typedef struct {
    int node_id;
    int num_choices;
    Choice choices[MAX_CHOICES];
} TreeNode;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char display_name[MAX_FILENAME_LENGTH];
} SaveFile;

// Global arrays to store game data
DialogEntry *dialogs = NULL;
TreeNode *tree_nodes = NULL;
int num_dialogs = 0;
int num_nodes = 0;

// Function prototypes
int load_dialog_file(const char *filename);
int load_tree_file(const char *filename);
TreeNode* find_node(int node_id);
DialogEntry* find_dialog(int dialog_id);
void play_game(int start_node);
void cleanup();
void clear_screen();
int show_main_menu();
int save_game(int current_node, const char *save_name);
int load_game(const char *save_name);
int list_save_files(SaveFile saves[], int max_saves);
void create_save_directory();
int file_exists(const char *filename);
int show_save_menu(int current_node);
int show_load_menu();

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
    
    if (menu_choice == 2) {
        // Load game
        start_node = show_load_menu();
        if (start_node == -1) {
            printf("No save files found or load cancelled.\n");
            printf("Starting new game...\n\n");
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

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    // Use ANSI escape sequences for better compatibility
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

int show_main_menu() {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║        ADVENTURE GAME            ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║                                  ║\n");
    printf("║  1) New Game                     ║\n");
    printf("║  2) Load Game                    ║\n");
    printf("║  3) Exit                         ║\n");
    printf("║                                  ║\n");
    printf("╚══════════════════════════════════╝\n");
    printf("\nEnter your choice (1-3): ");
    
    int choice;
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Invalid choice. Please enter 1, 2, or 3: ");
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (choice == 3) {
        printf("Thanks for playing!\n");
        exit(0);
    }
    
    return choice;
}

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

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
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
