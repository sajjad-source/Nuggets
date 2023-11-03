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
    addr_t playerAdd;
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
        if (i == size-1) {
            strncpy(gameMap->grid[i], lineBuffer, size+1);
            gameMap->grid[i][size] = '\0'; // Ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, size+2);
        gameMap->grid[i][size] = '\n'; // Ensure null termination
        gameMap->grid[i][size+1] = '\0'; // Ensure null termination
    }

    fclose(fp);
    return gameMap;
}

char* serialize_map(GameMap *gameMap) {
    // Calculate buffer size: one char for each cell plus one for each newline, plus one for the null terminator
    int bufferSize = gameMap->mapSize * (gameMap->mapSize + 1) + 1;
    char *buffer = malloc(bufferSize);
    if (buffer == NULL) {
        perror("Error allocating buffer for serialization");
        return NULL;
    }

    char* p = buffer;
    for (int i = 0; i < gameMap->mapSize; i++) {
        // Copy the row and add a newline character
        strncpy(p, gameMap->grid[i], gameMap->mapSize+1);
        p += gameMap->mapSize;
        // *p = '\n';
        p += 1;
    }
    *p = '\0'; // Null-terminate the buffer

    return buffer; 
}

bool handle_player_join(void* arg, const addr_t from, const char* buf) {
    // This handler will be called when a 'join' message is received.

    GameMap* game_map = (GameMap*)arg;

    printf("player %s joined @: %s\n", buf, message_stringAddr(from));

    Player* player = malloc(sizeof(Player));
    if (!player) {
        perror("Error allocating memory for player");
        free(player);
        return true;
    }

    strcpy(player->name, buf);
    player->playerAdd = from;

    message_send(from, serialize_map(game_map));

    game_map->players = player;

    // Return false if the message loop should continue, true if it should terminate.
    return false;
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
    }
    
    // Start the message loop, passing in handlers for different message types
    message_loop(game_map, 0, NULL, NULL, handle_player_join);

    // Cleanup
    message_done();

    free(game_map);
    return 0;
}
