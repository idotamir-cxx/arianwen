#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_CHOICES 10
#define MAX_TEXT_LENGTH 2048

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
    
    printf("Game loaded successfully!\n");
    printf("Starting adventure...\n\n");
    
    // Start the game from node 1 (assuming 1 is the starting node)
    play_game(1);
    
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
        
        // Get user input
        printf("\nEnter your choice (1-%d): ", node->num_choices);
        int choice;
        if (scanf("%d", &choice) != 1 || choice < 1 || choice > node->num_choices) {
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
        
        // Move to next node
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
