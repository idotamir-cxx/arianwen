# Makefile for Adventure Game

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET_ADVENTURE = adventure
TARGET_CHARACTER = character

# Source files for adventure game
ADVENTURE_SOURCES = main.c file_loader.c save_system.c character_system.c utils.c
ADVENTURE_OBJECTS = $(ADVENTURE_SOURCES:.c=.o)

# Source file for character creation
CHARACTER_SOURCES = character.c
CHARACTER_OBJECTS = $(CHARACTER_SOURCES:.c=.o)

# Header files
HEADERS = game_types.h file_loader.h save_system.h character_system.h utils.h

.PHONY: all clean

# Build both programs
all: $(TARGET_ADVENTURE) $(TARGET_CHARACTER)

# Adventure game executable
$(TARGET_ADVENTURE): $(ADVENTURE_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Character creation executable
$(TARGET_CHARACTER): $(CHARACTER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(ADVENTURE_OBJECTS) $(CHARACTER_OBJECTS) $(TARGET_ADVENTURE) $(TARGET_CHARACTER)

# Install (copy to /usr/local/bin - optional)
install: $(TARGET_ADVENTURE) $(TARGET_CHARACTER)
	cp $(TARGET_ADVENTURE) /usr/local/bin/
	cp $(TARGET_CHARACTER) /usr/local/bin/

# Create necessary directories
setup:
	mkdir -p saves characters

# Run the game (for testing)
run: $(TARGET_ADVENTURE)
	./$(TARGET_ADVENTURE) tree.txt dialog.txt

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build both adventure and character programs"
	@echo "  adventure - Build only the adventure game"
	@echo "  character - Build only the character creation program"
	@echo "  clean     - Remove all build files"
	@echo "  setup     - Create necessary directories"
	@echo "  run       - Build and run the adventure game"
	@echo "  install   - Install programs to /usr/local/bin"
	@echo "  help      - Show this help message"
