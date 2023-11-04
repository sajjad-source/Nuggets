#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support/message.h"

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
    int x;
    int y;
} Empty;

typedef struct {
    char** grid;
    GoldPile* gold_piles;
    int emptySpaceCount;
    Empty* emptySpaces;
    Player* players[27];
    int mapSize;
    int numGoldPiles;
    int goldLeft;
    int port;
} GameMap;

static int playerID = 0;
char characters[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

// Function declarations
GameMap* load_map(const char* map_filename);
GameMap* initialize_game(const char* map_filename);
void handle_player_join(GameMap* game_map, addr_t from, char* buf);
bool handle_player_quit(void* arg, const addr_t from, const char* buf);
void handle_player_move(GameMap* game_map, addr_t from, char* buf);


// This function is used to find empty spaces on the map
Empty* find_empty_spaces(char** grid, int size, int* count) {
    Empty* emptySpaces = malloc(size * size * sizeof(Empty));
    *count = 0; // Start with zero empty spaces found

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] == '.') {
                emptySpaces[*count].x = j;
                emptySpaces[*count].y = i;
                (*count)++;
            }
        }
    }

    // Shrink the array to the number of empty spaces found
    emptySpaces = realloc(emptySpaces, (*count) * sizeof(Empty));
    return emptySpaces;
}

void distribute_gold(GameMap* game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles) {

    // Seed the random number generator
    srand(10);

    // Determine the number of gold piles
    int numPiles = goldMinNumPiles + rand() % (goldMaxNumPiles - goldMinNumPiles + 1);

    // Allocate memory for the gold piles
    game_map->gold_piles = malloc(numPiles * sizeof(GoldPile));
    if (game_map->gold_piles == NULL) {
        perror("Error allocating memory for gold piles");
        return;
    }

    // Distribute gold among the piles
    int remainingGold = goldTotal;
    for (int i = 0; i < numPiles; i++) {
        // Ensure each pile gets at least 1 gold piece
        int goldInPile = (remainingGold > (numPiles - i)) ? (1 + rand() % (remainingGold - (numPiles - i))) : 1;
        game_map->gold_piles[i].gold_count = goldInPile;
        remainingGold -= goldInPile;
    }

    // Place gold piles on the map
    for (int i = 0; i < numPiles; i++) {
        if (i < game_map->emptySpaceCount) {

            int randomIndex = rand() % game_map->emptySpaceCount;

            game_map->gold_piles[i].position[0] = game_map->emptySpaces[randomIndex].x;
            game_map->gold_piles[i].position[1] = game_map->emptySpaces[randomIndex].y;

            game_map->emptySpaces[randomIndex] = game_map->emptySpaces[game_map->emptySpaceCount - 1];
            game_map->emptySpaceCount--;
        }
    }

    game_map->numGoldPiles = numPiles;
    game_map->goldLeft = goldTotal;
}

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
    char lineBuffer[size + 2];              // +2 for newline and null terminator
    for (int i = 0; i < size && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++) {
        if (i == size-1) {
            strncpy(gameMap->grid[i], lineBuffer, size+1);
            gameMap->grid[i][size] = '\0';  // Ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, size+2);
        gameMap->grid[i][size] = '\n';      // Ensure null termination
        gameMap->grid[i][size+1] = '\0';    // Ensure null termination
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

char* serialize_map_with_players(GameMap *gameMap, addr_t from) {
    // Calculate buffer size: one char for each cell plus one for each newline, plus one for the null terminator
    int bufferSize = gameMap->mapSize * (gameMap->mapSize + 1) + 100;
    char* buffer = malloc(bufferSize);
    
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

    char* playerInfo = malloc(bufferSize); // Assuming this is enough space for the player info

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
                    tempMap[player->position[1]][player->position[0]] = '@'; // Use the player's ID as the character
                    snprintf(playerInfo, bufferSize, "Player %c has %d nuggets (%d nuggets unclaimed).\n",
                     player->ID, player->gold_count, gameMap->goldLeft);
                }
            }
        }
    }

    if (gameMap->players[26] != NULL) {
        if (message_eqAddr(from, gameMap->players[26]->from)) {
            snprintf(playerInfo, bufferSize, "Spectator: %d nuggets unclaimed. Play at plank %d\n", gameMap->goldLeft, gameMap->port);
        }
    }

    // Overlay gold piles positions on the temporary map
    for (int i = 0; i < gameMap->numGoldPiles; i++) { // Assuming numGoldPiles is the number of gold piles
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
        strncpy(p, tempMap[i], gameMap->mapSize + 1); // Copy each row including the newline
        p += gameMap->mapSize + 1; // Move the pointer by the size of the row plus newline
    }
    *p = '\0'; // Null-terminate the buffer

    // Free the temporary map
    for (int i = 0; i < gameMap->mapSize; i++) {
        free(tempMap[i]);
    }
    free(tempMap);

    strcat(playerInfo, buffer);

    return playerInfo; // Return the serialized buffer
}



void handle_player_join(GameMap* game_map, addr_t from, char* player_name) {

    if (strcmp(player_name, "Spectator") == 0) {

        Player* player = malloc(sizeof(Player));

        if (!player) {
            perror("Error allocating memory for player");
            return; // Exit function since we can't do anything without memory
        }
        strcpy(player->name, player_name);

        player->from = from; // Assign the address from which the player joined

        game_map->players[26] = player;

        return;
    }

    if (playerID < 26) { // Check if there is space for a new player
        Player* player = malloc(sizeof(Player));
        if (!player) {
            perror("Error allocating memory for player");
            return; // Exit function since we can't do anything without memory
        }
        strcpy(player->name, player_name);

        // Assign a unique ID to the player based on playerID
        player->ID = characters[playerID]; 
        player->from = from; // Assign the address from which the player joined

        // Initialize other fields if necessary
        player->gold_count = 0;

        if (game_map->emptySpaceCount > 0) {
            int randomIndex = rand() % game_map->emptySpaceCount;

            player->position[0] = game_map->emptySpaces[randomIndex].x; 
            player->position[1] = game_map->emptySpaces[randomIndex].y;

            game_map->emptySpaces[randomIndex] = game_map->emptySpaces[game_map->emptySpaceCount - 1];
            game_map->emptySpaceCount--;
        }

        printf("Player %s joined with ID %c\n", player_name, player->ID);
        game_map->players[playerID] = player;
        playerID++; // Increment the playerID for the next player
    } else {
        printf("Max players reached. Cannot add more players.\n");
    }
}

void handle_player_move(GameMap* game_map, addr_t from, char* moveDirectionStr) {

    // Extract move direction from buf
    char moveDirection = moveDirectionStr[0];

    // Find the player with the given ID
    Player* player = NULL;
    for (int i = 0; i < 26; i++) {
        if (game_map->players[i] && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];
            break;
        }
    }

    // If the player is not found, return
    if (player == NULL) {
        fprintf(stderr, "Player not found for the given address.\n");
        return;
    }

    // Calculate new position based on move direction
    int newRow = player->position[1];
    int newCol = player->position[0];
    switch (moveDirection) {
        case 'h':
        newCol--;
        break; // Move left
        case 'l':
        newCol++;
        break; // Move right
        case 'j':
        newRow++;
        break; // Move down
        case 'k':
        newRow--;
        break; // Move up
        case 'y':
        newRow--;
        newCol--;
        break; // Move diagonally up-left
        case 'u':
        newRow--;
        newCol++;
        break; // Move diagonally up-right
        case 'b':
        newRow++;
        newCol--;
        break; // Move diagonally down-left
        case 'n':
        newRow++;
        newCol++;
        break; // Move diagonally down-right
        default:
            fprintf(stderr, "Invalid move direction %c.\n", moveDirection);
            return;
    }

    // Check if the new position is within the bounds of the map and not occupied
    if (newRow >= 0 && newRow < game_map->mapSize && newCol >= 0 && newCol < game_map->mapSize && (game_map->grid[newRow][newCol] == '.' || game_map->grid[newRow][newCol] == '#')) { // Assuming '.' represents an open space
        // Update player's position
        player->position[1] = newRow;
        player->position[0] = newCol;

        // If the new position is a gold pile, update the gold count and remove the pile
        for (int i = 0; i < game_map->numGoldPiles; i++) {
            if (game_map->gold_piles[i].position[0] == newCol && game_map->gold_piles[i].position[1] == newRow) {
                // Increase player's gold count
                player->gold_count += game_map->gold_piles[i].gold_count;
                game_map->goldLeft -= game_map->gold_piles[i].gold_count;

                // Remove the gold pile by swapping it with the last one in the array (if not already the last)
                if (i < game_map->numGoldPiles - 1) {
                    game_map->gold_piles[i] = game_map->gold_piles[game_map->numGoldPiles - 1];
                }

                // Decrease the count of gold piles
                game_map->numGoldPiles--;

                printf("over gold");

                break; // Exit the loop after handling the gold pile
            }
        }
    } else {
        fprintf(stderr, "Invalid move. Position out of bounds or occupied.\n");
    }
}



bool handleMessage(void* arg, const addr_t from, const char* buf) {
    GameMap* game_map = (GameMap*)arg;
    char* message = strdup(buf); // Duplicate the buffer to use with strtok

    if (message != NULL) {
        char* command = strtok(message, " ");

        if (command != NULL && strcmp(command, "Join") == 0) {
            char* playerName = strtok(NULL, " ");
            if (playerName != NULL && strcmp(playerName, "Spectator") != 0) {
                handle_player_join(game_map, from, playerName);
            } else {
                handle_player_join(game_map, from, "Spectator");
            }
        } else {
            printf("Unknown command received.\n");
        } 
        
        if (strcmp(command, "Move") == 0) {
            char* moveDirection = strtok(NULL, " ");
            if (moveDirection != NULL) {
            handle_player_move(game_map, from, moveDirection);
            }
        } else {
            printf("Unknown command received.\n");
        }

        printf("Message %s from: %s\n", buf, message_stringAddr(from));

        free(message); // Free the duplicated message buffer
    }


    for (int i = 0; i <= 26; i++) {
        if (game_map->players[i] != NULL) {
            // Use serialize_map_with_players to account for players on the map
            char* serializedMap = serialize_map_with_players(game_map, game_map->players[i]->from);
            if (serializedMap != NULL) {
                message_send(game_map->players[i]->from, serializedMap);
                free(serializedMap); // Remember to free the serialized map
            }
        }
    }

    return false; // Continue the message loop
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

    game_map->port = port;
    
    // Start the message loop, passing in handlers for different message types
    message_loop(game_map, 0, NULL, NULL, handleMessage);

    // Cleanup
    message_done();

    free(game_map);
    return 0;
}
