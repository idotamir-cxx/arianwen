#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_NAME_LENGTH 50
#define MAX_CLASS_LENGTH 20
#define MAX_ALIGNMENT_LENGTH 15
#define MAX_FILENAME_LENGTH 256
#define CHARACTER_DIR "characters"

typedef struct {
    char name[MAX_NAME_LENGTH];
    char class[MAX_CLASS_LENGTH];
    char alignment[MAX_ALIGNMENT_LENGTH];
    int hit_points;
    int max_hit_points;
} Character;

// Function prototypes
void clear_screen();
int create_character(Character *character);
int save_character(const Character *character, const char *filename);
int load_character(Character *character, const char *filename);
void create_character_directory();
int stage_name(Character *character);
int stage_class(Character *character);
int stage_alignment(Character *character);
int stage_summary(Character *character);
void display_character(const Character *character);
int get_hit_points_for_class(const char *class);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <character_filename>\n", argv[0]);
        return 1;
    }
    
    Character character;
    
    // Create character directory if it doesn't exist
    create_character_directory();
    
    // Create new character
    if (create_character(&character) != 0) {
        printf("Character creation cancelled.\n");
        return 1;
    }
    
    // Save character
    if (save_character(&character, argv[1]) != 0) {
        printf("Error saving character!\n");
        return 1;
    }
    
    printf("Character created and saved successfully!\n");
    return 0;
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

void create_character_directory() {
    struct stat st = {0};
    if (stat(CHARACTER_DIR, &st) == -1) {
#ifdef _WIN32
        mkdir(CHARACTER_DIR);
#else
        mkdir(CHARACTER_DIR, 0700);
#endif
    }
}

int create_character(Character *character) {
    // Initialize character
    memset(character, 0, sizeof(Character));
    
    int stage = 1;  // 1=name, 2=class, 3=alignment, 4=summary
    
    while (stage >= 1 && stage <= 4) {
        switch (stage) {
            case 1:
                stage = stage_name(character);
                break;
            case 2:
                stage = stage_class(character);
                break;
            case 3:
                stage = stage_alignment(character);
                break;
            case 4:
                stage = stage_summary(character);
                break;
        }
    }
    
    return (stage == 5) ? 0 : -1;  // 5 means completed, anything else is cancelled
}

int stage_name(Character *character) {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║      CHARACTER CREATION          ║\n");
    printf("║         STEP 1: NAME             ║\n");
    printf("╚══════════════════════════════════╝\n\n");
    
    if (strlen(character->name) > 0) {
        printf("Current name: %s\n\n", character->name);
    }
    
    printf("Enter your character's name: ");
    char name[MAX_NAME_LENGTH];
    
    if (fgets(name, sizeof(name), stdin) == NULL) {
        return -1;
    }
    
    // Remove newline
    char *newline = strchr(name, '\n');
    if (newline) *newline = '\0';
    
    // Check for empty name
    if (strlen(name) == 0) {
        printf("Name cannot be empty. Press Enter to try again...");
        getchar();
        return 1;  // Stay on same stage
    }
    
    strncpy(character->name, name, MAX_NAME_LENGTH - 1);
    character->name[MAX_NAME_LENGTH - 1] = '\0';
    
    printf("\nName set to: %s\n", character->name);
    printf("\n1) Accept and continue\n");
    printf("2) Enter different name\n");
    printf("3) Cancel character creation\n");
    printf("\nChoice (1-3): ");
    
    int choice;
    scanf("%d", &choice);
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    switch (choice) {
        case 1: return 2;   // Next stage
        case 2: return 1;   // Same stage
        case 3: return -1;  // Cancel
        default:
            printf("Invalid choice. Press Enter to continue...");
            getchar();
            return 1;  // Stay on same stage
    }
}

int stage_class(Character *character) {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║      CHARACTER CREATION          ║\n");
    printf("║        STEP 2: CLASS             ║\n");
    printf("╚══════════════════════════════════╝\n\n");
    
    printf("Character: %s\n\n", character->name);
    
    if (strlen(character->class) > 0) {
        printf("Current class: %s (HP: %d)\n\n", character->class, character->hit_points);
    }
    
    printf("Choose your class:\n");
    printf("1) Fighter    (8 hit points)\n");
    printf("2) Wizard     (4 hit points)\n");
    printf("3) Cleric     (6 hit points)\n");
    printf("4) Thief      (4 hit points)\n");
    printf("5) Elf        (6 hit points)\n");
    printf("6) Halfling   (8 hit points)\n");
    printf("7) Dwarf      (8 hit points)\n");
    printf("\n8) Go back to name\n");
    printf("9) Cancel character creation\n");
    
    printf("\nChoice (1-9): ");
    
    int choice;
    scanf("%d", &choice);
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    const char* classes[] = {"Fighter", "Wizard", "Cleric", "Thief", "Elf", "Halfling", "Dwarf"};
    
    if (choice >= 1 && choice <= 7) {
        strncpy(character->class, classes[choice - 1], MAX_CLASS_LENGTH - 1);
        character->class[MAX_CLASS_LENGTH - 1] = '\0';
        character->hit_points = get_hit_points_for_class(character->class);
        character->max_hit_points = character->hit_points;
        
        printf("\nClass set to: %s (HP: %d)\n", character->class, character->hit_points);
        printf("\n1) Accept and continue\n");
        printf("2) Choose different class\n");
        printf("3) Go back to name\n");
        printf("4) Cancel character creation\n");
        printf("\nChoice (1-4): ");
        
        int confirm;
        scanf("%d", &confirm);
        
        // Clear input buffer
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (confirm) {
            case 1: return 3;   // Next stage
            case 2: return 2;   // Same stage
            case 3: return 1;   // Previous stage
            case 4: return -1;  // Cancel
            default:
                printf("Invalid choice. Press Enter to continue...");
                getchar();
                return 2;  // Stay on same stage
        }
    } else if (choice == 8) {
        return 1;  // Previous stage
    } else if (choice == 9) {
        return -1; // Cancel
    } else {
        printf("Invalid choice. Press Enter to continue...");
        getchar();
        return 2;  // Stay on same stage
    }
}

int stage_alignment(Character *character) {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║      CHARACTER CREATION          ║\n");
    printf("║       STEP 3: ALIGNMENT          ║\n");
    printf("╚══════════════════════════════════╝\n\n");
    
    printf("Character: %s the %s (HP: %d)\n\n", 
           character->name, character->class, character->hit_points);
    
    if (strlen(character->alignment) > 0) {
        printf("Current alignment: %s\n\n", character->alignment);
    }
    
    printf("Choose your alignment:\n");
    printf("1) Lawful  - Believes in order and structure\n");
    printf("2) Neutral - Balanced, pragmatic approach\n");
    printf("3) Evil    - Selfish, ruthless tendencies\n");
    printf("\n4) Go back to class\n");
    printf("5) Cancel character creation\n");
    
    printf("\nChoice (1-5): ");
    
    int choice;
    scanf("%d", &choice);
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    const char* alignments[] = {"Lawful", "Neutral", "Evil"};
    
    if (choice >= 1 && choice <= 3) {
        strncpy(character->alignment, alignments[choice - 1], MAX_ALIGNMENT_LENGTH - 1);
        character->alignment[MAX_ALIGNMENT_LENGTH - 1] = '\0';
        
        printf("\nAlignment set to: %s\n", character->alignment);
        printf("\n1) Accept and continue\n");
        printf("2) Choose different alignment\n");
        printf("3) Go back to class\n");
        printf("4) Cancel character creation\n");
        printf("\nChoice (1-4): ");
        
        int confirm;
        scanf("%d", &confirm);
        
        // Clear input buffer
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (confirm) {
            case 1: return 4;   // Next stage
            case 2: return 3;   // Same stage
            case 3: return 2;   // Previous stage
            case 4: return -1;  // Cancel
            default:
                printf("Invalid choice. Press Enter to continue...");
                getchar();
                return 3;  // Stay on same stage
        }
    } else if (choice == 4) {
        return 2;  // Previous stage
    } else if (choice == 5) {
        return -1; // Cancel
    } else {
        printf("Invalid choice. Press Enter to continue...");
        getchar();
        return 3;  // Stay on same stage
    }
}

int stage_summary(Character *character) {
    clear_screen();
    printf("╔══════════════════════════════════╗\n");
    printf("║      CHARACTER CREATION          ║\n");
    printf("║        STEP 4: SUMMARY           ║\n");
    printf("╚══════════════════════════════════╝\n\n");
    
    display_character(character);
    
    printf("\n1) Create this character\n");
    printf("2) Go back to alignment\n");
    printf("3) Cancel character creation\n");
    
    printf("\nChoice (1-3): ");
    
    int choice;
    scanf("%d", &choice);
    
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    switch (choice) {
        case 1: return 5;   // Complete (success)
        case 2: return 3;   // Previous stage
        case 3: return -1;  // Cancel
        default:
            printf("Invalid choice. Press Enter to continue...");
            getchar();
            return 4;  // Stay on same stage
    }
}

void display_character(const Character *character) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║            CHARACTER SHEET            ║\n");
    printf("╠═══════════════════════════════════════╣\n");
    printf("║                                       ║\n");
    printf("║  Name:      %-24s ║\n", character->name);
    printf("║  Class:     %-24s ║\n", character->class);
    printf("║  Alignment: %-24s ║\n", character->alignment);
    printf("║  Hit Points: %2d/%2d                   ║\n", 
           character->hit_points, character->max_hit_points);
    printf("║                                       ║\n");
    printf("╚═══════════════════════════════════════╝\n");
}

int get_hit_points_for_class(const char *class) {
    if (strcmp(class, "Fighter") == 0) return 8;
    if (strcmp(class, "Wizard") == 0) return 4;
    if (strcmp(class, "Cleric") == 0) return 6;
    if (strcmp(class, "Thief") == 0) return 4;
    if (strcmp(class, "Elf") == 0) return 6;
    if (strcmp(class, "Halfling") == 0) return 8;
    if (strcmp(class, "Dwarf") == 0) return 8;
    return 4;  // Default fallback
}

int save_character(const Character *character, const char *filename) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", CHARACTER_DIR, filename);
    
    FILE *file = fopen(full_path, "w");
    if (!file) {
        return -1;
    }
    
    fprintf(file, "NAME:%s\n", character->name);
    fprintf(file, "CLASS:%s\n", character->class);
    fprintf(file, "ALIGNMENT:%s\n", character->alignment);
    fprintf(file, "HP:%d\n", character->hit_points);
    fprintf(file, "MAX_HP:%d\n", character->max_hit_points);
    
    fclose(file);
    return 0;
}

int load_character(Character *character, const char *filename) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", CHARACTER_DIR, filename);
    
    FILE *file = fopen(full_path, "r");
    if (!file) {
        return -1;
    }
    
    char line[256];
    memset(character, 0, sizeof(Character));
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char *colon = strchr(line, ':');
        if (!colon) continue;
        
        *colon = '\0';
        char *key = line;
        char *value = colon + 1;
        
        if (strcmp(key, "NAME") == 0) {
            strncpy(character->name, value, MAX_NAME_LENGTH - 1);
        } else if (strcmp(key, "CLASS") == 0) {
            strncpy(character->class, value, MAX_CLASS_LENGTH - 1);
        } else if (strcmp(key, "ALIGNMENT") == 0) {
            strncpy(character->alignment, value, MAX_ALIGNMENT_LENGTH - 1);
        } else if (strcmp(key, "HP") == 0) {
            character->hit_points = atoi(value);
        } else if (strcmp(key, "MAX_HP") == 0) {
            character->max_hit_points = atoi(value);
        }
    }
    
    fclose(file);
    return 0;
}
