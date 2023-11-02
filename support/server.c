#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

// Major Data Structures
typedef struct {
    char ID;
    char name[50];
    int position[2];
    int gold_count;
    char** visible_grid;
} Player;

typedef struct {
    int position[2];
    int gold_count;
} GoldPile;

typedef struct {
    char** grid;
    GoldPile* gold_piles;
    Player* players;
    int mapSize;
} GameMap;

// Function declarations
GameMap* load_map(const char* map_filename);
GameMap* initialize_game(const char* map_filename);
bool handle_player_join(void* arg, const addr_t from, const char* buf);
bool handle_player_quit(void* arg, const addr_t from, const char* buf);
bool handle_player_move(void* arg, const addr_t from, const char* buf);

GameMap* initialize_game(const char* map_filename) {
    // Open the map file
    FILE* fp = fopen(map_filename, "r");
    if (fp == NULL) {
        perror("Error opening map file");
        return NULL;
    }

    // Assuming the map is always square, count the number of characters until a newline
    int size = 0; // This will be the size for both rows and columns
    while (fgetc(fp) != '\n') {
        size++;
    }

    // Rewind to the start of the file to read the grid
    rewind(fp);

    // Allocate memory for the GameMap structure
    GameMap* gameMap = malloc(sizeof(GameMap));
    if (gameMap == NULL) {
        perror("Error allocating memory for game map");
        fclose(fp);
        return NULL;
    }

    gameMap->mapSize = size;

    // Allocate memory for the grid
    gameMap->grid = malloc(size * sizeof(char*));
    if (gameMap->grid == NULL) {
        perror("Error allocating memory for grid");
        free(gameMap);
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        gameMap->grid[i] = malloc((size + 2) * sizeof(char)); // +1 for new line and null terminator
        if (gameMap->grid[i] == NULL) {
            // Handle error, free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(gameMap->grid[j]);
            }
            free(gameMap->grid);
            free(gameMap);
            fclose(fp);
            return NULL;
        }
    }

    // Read the map from the file
    char lineBuffer[size + 2]; // +2 for newline and null terminator
    for (int i = 0; i < size && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++) {
        strncpy(gameMap->grid[i], lineBuffer, size+2);
        gameMap->grid[i][size] = '\n'; // Ensure null termination
        gameMap->grid[i][size+1] = '\0'; // Ensure null termination
    }

    fclose(fp);
    return gameMap;
}

bool handle_player_join(void* arg, const addr_t from, const char* buf) {
    // This handler will be called when a 'join' message is received.

    printf("joined @: %s", message_stringAddr(from));

    // Return true if the message loop should continue, false if it should terminate.
    return true;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./server <map_filename>\n");
        return 1;
    }
    
    // Initialize the messaging system and start listening on a port
    int port = message_init(stderr);
    if (port == 0) {
        fprintf(stderr, "Failed to initialize messaging system.\n");
        return 1;
    }

    // Load the game map and initialize the game state
    GameMap* game_map = initialize_game(argv[1]);
    if (game_map == NULL) {
        fprintf(stderr, "Failed to initialize game.\n");
        return 1;
    }

    for (int i = 0; i < game_map->mapSize; i++) {
        printf("%s", game_map->grid[i]);
        // printf("%d", game_map->mapSize);
    }
    
    // Start the message loop, passing in handlers for different message types
    message_loop(game_map, 0, NULL, NULL, handle_player_join);

    // Cleanup
    message_done();

    free(game_map);
    return 0;
}
