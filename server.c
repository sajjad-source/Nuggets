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
    addr_t from;
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
    int emptySpaceCount;
    Empty* emptySpaces;
    Player* players[26];
    int mapSize;
    int numGoldPiles;

} GameMap;

typedef struct {
    int x;
    int y;
} Empty;

static int playerID = 0;
char playerLetter[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

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
    char lineBuffer[size + 2];                              // +2 for newline and null terminator
    for (int i = 0; i < size && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++) {
        if (i == size-1) {
            strncpy(gameMap->grid[i], lineBuffer, size+1);
            gameMap->grid[i][size] = '\0';                  // Ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, size+2);
        gameMap->grid[i][size] = '\n';                      // Ensure null termination
        gameMap->grid[i][size+1] = '\0';                    // Ensure null termination
    }

    // Find empty spaces after loading the map
    int emptySpaceCount;
    Empty* emptySpaces = find_empty_spaces(gameMap->grid, gameMap->mapSize, &emptySpaceCount);

    gameMap->emptySpaceCount = emptySpaceCount;

    gameMap->emptySpaces = emptySpaces;

    if (emptySpaceCount == 0) {
        fprintf(stderr, "No empty spaces found on the map.\n");
    }

    distribute_gold(gameMap, 250, 10, 30);

    fclose(fp);
    return gameMap;
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

    message_send(from, serialize_map(game_map));

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

char* serialize_map_with_players(GameMap *gameMap, addr_t from) {
    // Calculate buffer size: one char for each cell plus one for each newline, plus one for the null terminator
    int bufferSize = gameMap->mapSize * (gameMap->mapSize + 1) + 1;
    char *buffer = malloc(bufferSize);
    
    if (buffer == NULL) {
        perror("Error allocating buffer for serialization");
        return NULL;
    }

    // Create a copy of the map
    char** tempMap = malloc(gameMap->mapSize * sizeof(char*));
    for (int i = 0; i < gameMap->mapSize; i++) {
        tempMap[i] = strdup(gameMap->grid[i]); // Copy each row
        if (tempMap[i] == NULL) {
            // Handle allocation failure; free previously allocated strings and the array, then return NULL
            for (int j = 0; j < i; j++) {
                free(tempMap[j]);
            }
            free(tempMap);
            free(buffer);
            return NULL;
        }
    }

    // Overlay players' positions on the temporary map
    for (int i = 0; i < 26; i++) {
        Player* player = gameMap->players[i];
        if (player != NULL) {
            // Ensure the position is within the bounds of the map
            if (player->position[0] >= 0 && player->position[0] < gameMap->mapSize &&
                player->position[1] >= 0 && player->position[1] < gameMap->mapSize) {
                if (!message_eqAddr(player->from, from)) {
                    tempMap[player->position[1]][player->position[0]] = player->ID; // Use the player's ID as the character
                } else {
                    tempMap[player->position[1]][player->position[0]] = '@';        // Use the player's ID as the character
                }
            }
        }
    }

    // Overlay gold piles positions on the temporary map
    for (int i = 0; i < gameMap->numGoldPiles; i++) {                               // Assuming numGoldPiles is the number of gold piles
        GoldPile goldPile = gameMap->gold_piles[i];
        // Ensure the position is within the bounds of the map
        if (goldPile.position[0] >= 0 && goldPile.position[0] < gameMap->mapSize &&
            goldPile.position[1] >= 0 && goldPile.position[1] < gameMap->mapSize) {
            tempMap[goldPile.position[1]][goldPile.position[0]] = '*';
        }
    }

    // Serialize the temporary map into the buffer
    char* p = buffer;
    for (int i = 0; i < gameMap->mapSize; i++) {
        strncpy(p, tempMap[i], gameMap->mapSize + 1);                               // Copy each row including the newline
        p += gameMap->mapSize + 1;                                                  // Move the pointer by the size of the row plus newline
    }
    *p = '\0';                                                                      // Null-terminate the buffer

    // Free the temporary map
    for (int i = 0; i < gameMap->mapSize; i++) {
        free(tempMap[i]);
    }
    free(tempMap);

    return buffer;                                                                  // Return the serialized buffer
}