#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "game_types.h"
#include "character_system.h"
#include "utils.h"

int create_new_character() {
    // Call external character creation program
    printf("Creating new character...\n\n");

    char char_filename[MAX_FILENAME_LENGTH];
    printf("Enter character filename (without extension): ");

    if (fgets(char_filename, sizeof(char_filename), stdin) == NULL) {
        return -1;
    }

    // Remove newline
    char *newline = strchr(char_filename, '\n');
    if (newline) *newline = '\0';

    // Add .char extension
    strcat(char_filename, ".char");

    // Create character directory
    struct stat st = {0};
    if (stat(CHARACTER_DIR, &st) == -1) {
#ifdef _WIN32
        mkdir(CHARACTER_DIR);
#else
        mkdir(CHARACTER_DIR, 0700);
#endif
    }

    // Run character creation program
    char command[512];
    snprintf(command, sizeof(command), "./character %s", char_filename);

    if (system(command) != 0) {
        printf("Error running character creation. Make sure 'character' executable exists.\n");
        printf("Creating basic character instead...\n");

        // Create a basic default character
        strcpy(current_character.name, "Adventurer");
        strcpy(current_character.class, "Fighter");
        strcpy(current_character.alignment, "Neutral");
        current_character.hit_points = 8;
        current_character.max_hit_points = 8;

        // Save it
        if (save_character(&current_character, char_filename) != 0) {
            printf("Failed to save character.\n");
            return -1;
        }
    } else {
        // Load the created character
        if (load_character(&current_character, char_filename) != 0) {
            printf("Failed to load created character.\n");
            return -1;
        }
    }

    printf("Character loaded successfully!\n");
    return 0;
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
    fprintf(file, "STR:%d\n", character->abilities.strength);
    fprintf(file, "INT:%d\n", character->abilities.intelligence);
    fprintf(file, "WIS:%d\n", character->abilities.wisdom);
    fprintf(file, "DEX:%d\n", character->abilities.dexterity);
    fprintf(file, "CON:%d\n", character->abilities.constitution);
    fprintf(file, "CHA:%d\n", character->abilities.charisma);

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
        } else if (strcmp(key, "STR") == 0) {
            character->abilities.strength = atoi(value);
        } else if (strcmp(key, "INT") == 0) {
            character->abilities.intelligence = atoi(value);
        } else if (strcmp(key, "WIS") == 0) {
            character->abilities.wisdom = atoi(value);
        } else if (strcmp(key, "DEX") == 0) {
            character->abilities.dexterity = atoi(value);
        } else if (strcmp(key, "CON") == 0) {
            character->abilities.constitution = atoi(value);
        } else if (strcmp(key, "CHA") == 0) {
            character->abilities.charisma = atoi(value);
        }
    }

    fclose(file);
    return 0;
}

void display_character_status() {
    printf("┌──────────────────────────────────────────────────────────────────┐\n");
    printf("│ %s the %s\n", current_character.name, current_character.class);
    printf("│ %s │ HP: %d/%d\n", current_character.alignment,
           current_character.hit_points, current_character.max_hit_points);
    printf("│ STR:%2d INT:%2d WIS:%2d DEX:%2d CON:%2d CHA:%2d │\n",
           current_character.abilities.strength, current_character.abilities.intelligence,
           current_character.abilities.wisdom, current_character.abilities.dexterity,
           current_character.abilities.constitution, current_character.abilities.charisma);
    printf("└──────────────────────────────────────────────────────────────────┘\n\n");
}