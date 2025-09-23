#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#define MAX_LINE_LENGTH 1024
#define MAX_CHOICES 10
#define MAX_TEXT_LENGTH 2048
#define MAX_FILENAME_LENGTH 256
#define MAX_SAVES 20
#define SAVE_DIR "saves"
#define CHARACTER_DIR "characters"
#define MAX_NAME_LENGTH 50
#define MAX_CLASS_LENGTH 20
#define MAX_ALIGNMENT_LENGTH 15

typedef struct {
    int strength;
    int intelligence;
    int wisdom;
    int dexterity;
    int constitution;
    int charisma;
} AbilityScores;

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

typedef struct {
    char name[MAX_NAME_LENGTH];
    char class[MAX_CLASS_LENGTH];
    char alignment[MAX_ALIGNMENT_LENGTH];
    int hit_points;
    int max_hit_points;
    AbilityScores abilities;
} Character;

// Global variables (declared here, defined in main.c)
extern DialogEntry *dialogs;
extern TreeNode *tree_nodes;
extern Character current_character;
extern int num_dialogs;
extern int num_nodes;

#endif