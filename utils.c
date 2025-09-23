#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    // Use ANSI escape sequences for better compatibility
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
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
