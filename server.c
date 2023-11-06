#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support/message.h"
#include "math.h"

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
    Player* players[26];
    int mapSize;
    int numGoldPiles;
} GameMap;

static int playerID = 1;
char characters[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };

// Function declarations
GameMap* initialize_game(const char* map_filename);
void handle_player_join(GameMap* game_map, addr_t from, char* buf);
void handle_player_move(GameMap* game_map, addr_t from, char* buf);
void calculate_visibility(GameMap* game_map, Player* player);
bool line_of_sight(GameMap* game_map, int x0, int y0, int x1, int y1);
bool is_clear_path(GameMap* game_map, int x0, int y0, int x1, int y1);
bool handleMessage(void* arg, const addr_t from, const char* buf);

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
    message_loop(game_map, 0, NULL, NULL, handleMessage);

    // Cleanup
    message_done();

    free(game_map);
    return 0;
}

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
    // Find the current player based on 'from' address
    Player* current_player = NULL;
    for (int i = 0; i < 26; i++) {
        if (gameMap->players[i] && message_eqAddr(gameMap->players[i]->from, from)) {
            current_player = gameMap->players[i];
            break;
        }
    }

    // If the current player is not found, return NULL
    if (current_player == NULL) {
        fprintf(stderr, "Player not found for the given address.\n");
        return NULL;
    }

    // Calculate visibility for the current player
    calculate_visibility(gameMap, current_player);

    // Allocate buffer for serialization
    int bufferSize = gameMap->mapSize * (gameMap->mapSize + 1) + 1;
    char *buffer = malloc(bufferSize);
    if (buffer == NULL) {
        perror("Error allocating buffer for serialization");
        return NULL;
    }

    // Copy the visible elements of the map to the buffer
    char* p = buffer;
    for (int y = 0; y < gameMap->mapSize; y++) {
        for (int x = 0; x < gameMap->mapSize; x++) {
            // Check if the current position is visible to the player
            if (current_player->visible_grid[y][x] != ' ') {
                // Overlay players' positions on the visible map
                bool player_here = false;
                for (int i = 0; i < 26; i++) {
                    Player* player = gameMap->players[i];
                    if (player && player->position[0] == x && player->position[1] == y) {
                        if (!message_eqAddr(player->from, from)) {
                            *p++ = player->ID; // Other player's ID
                        } else {
                            *p++ = '@'; // Current player
                        }
                        player_here = true;
                        break;
                    }
                }

                // Overlay gold piles if no player is present at this position
                if (!player_here) {
                    bool gold_here = false;
                    for (int i = 0; i < gameMap->numGoldPiles; i++) {
                        GoldPile goldPile = gameMap->gold_piles[i];
                        if (goldPile.position[0] == x && goldPile.position[1] == y) {
                            *p++ = '*'; // Gold pile
                            gold_here = true;
                            break;
                        }
                    }
                    // If no player or gold is present, copy the map character
                    if (!gold_here) {
                        *p++ = current_player->visible_grid[y][x];
                    }
                }
            } else {
                *p++ = ' '; // Non-visible spaces remain blank
            }
        }
        *p++ = '\n'; // Add newline at the end of each row
    }
    *p = '\0'; // Null-terminate the buffer

    // Free the visibility grid allocated in calculate_visibility
    for (int i = 0; i < gameMap->mapSize; i++) {
        free(current_player->visible_grid[i]);
    }
    free(current_player->visible_grid);
    current_player->visible_grid = NULL;

    return buffer; // Return the serialized buffer
}


void handle_player_join(GameMap* game_map, addr_t from, char* player_name) {
    if (playerID < 26) { // Check if there is space for a new player
        Player* player = malloc(sizeof(Player));
        if (!player) {
            perror("Error allocating memory for player");
            return; // Exit function since we can't do anything without memory
        }
        strcpy(player->name, player_name);

        // Assign a unique ID to the player based on playerID
        player->ID = characters[playerID-1]; 
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
        if (game_map->players[i] &&  message_eqAddr(game_map->players[i]->from, from)) {
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
    if (newRow >= 0 && newRow < game_map->mapSize && newCol >= 0 && newCol < game_map->mapSize && (game_map->grid[newRow][newCol] == '.' || game_map->grid[newRow][newCol] == '#' || game_map->grid[newRow][newCol] == '*')) { // Assuming '.' represents an open space
        // Update player's position
        player->position[1] = newRow;
        player->position[0] = newCol;

        // If the new position is a gold pile, update the gold count and remove the pile
        for (int i = 0; i < game_map->numGoldPiles; i++) {
            if (game_map->gold_piles[i].position[0] == newCol && game_map->gold_piles[i].position[1] == newRow) {
                // Increase player's gold count
                player->gold_count += game_map->gold_piles[i].gold_count;

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
            if (playerName != NULL) {
                handle_player_join(game_map, from, playerName);
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

    for (int i = 0; i < 26; i++) {
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

bool is_clear_path(GameMap* game_map, int start_x, int start_y, int end_x, int end_y) {
    float delta_x = abs(end_x - start_x);
    float delta_y = -abs(end_y - start_y);
    float step_x = start_x < end_x ? 1 : -1;
    float step_y = start_y < end_y ? 1 : -1;
    float error = delta_x + delta_y; // The error accumulator

    while (true) {
        // Check if we've reached the target point
        if (start_x == end_x && start_y == end_y) {
            // Check if the current position is a wall
            char current_pos = game_map->grid[start_y][start_x];
            if (current_pos == '-' || current_pos == '|' || current_pos == '+') {
                return true;        // The wall is visible
            }
            break;                  // Reached the target, end the loop
        }

        // Check if the current position is a clear path
        if (game_map->grid[start_y][start_x] != '.' && game_map->grid[start_y][start_x] != ' ') {
            return false;           // It's not a clear path
        }

        int error2 = 2 * error;
        if (error2 >= delta_y) {
            error += delta_y;       // Adjust the error
            start_x += step_x;      // Take a step in x
        }
        if (error2 <= delta_x) {
            error += delta_x;       // Adjust the error
            start_y += step_y;      // Take a step in y
        }
    }

    // Clear path if we've reached this point
    return true;
}

bool line_of_sight(GameMap* game_map, int pr, int pc, int r, int c) {
    // Directly adjacent spots are always visible
    if (abs(pr - r) <= 1 && abs(pc - c) <= 1) {
        return true;
    }

    // Check if there is a clear path to the target spot
    return is_clear_path(game_map, pr, pc, r, c);
}

void calculate_visibility(GameMap* game_map, Player* player) {
    // Create a visible grid for the player with the same size as the game_map
    player->visible_grid = malloc(game_map->mapSize * sizeof(char*));
    for (int i = 0; i < game_map->mapSize; i++) {
        player->visible_grid[i] = malloc((game_map->mapSize + 1) * sizeof(char));
        memset(player->visible_grid[i], ' ', game_map->mapSize);
        player->visible_grid[i][game_map->mapSize] = '\0'; // Null-terminate the string
    }

    // Check visibility for every cell in the grid
    for (int y = 0; y < game_map->mapSize; y++) {
        for (int x = 0; x < game_map->mapSize; x++) {
            // For walls, check adjacent tiles as well as direct line of sight
            if (game_map->grid[y][x] == '-' || game_map->grid[y][x] == '|' || game_map->grid[y][x] == '+') {
                bool visible = false;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int checkX = x + dx;
                        int checkY = y + dy;
                        // Ensure that we don't check out of bounds or through corners
                        if (checkY >= 0 && checkY < game_map->mapSize && checkX >= 0 && checkX < game_map->mapSize) {
                            // Diagonal checks are through corners, so we skip them
                            if (dx != 0 && dy != 0) continue;
                            if (line_of_sight(game_map, player->position[0], player->position[1], checkX, checkY)) {
                                visible = true;
                                break; // Break out of the inner loop on first visible
                            }
                        }
                    }
                    if (visible) break; // Break out of the outer loop if visible
                }
                if (visible) {
                    player->visible_grid[y][x] = game_map->grid[y][x];
                }
            }
            // For non-wall tiles, use direct line of sight check
            else if (line_of_sight(game_map, player->position[0], player->position[1], x, y)) {
                player->visible_grid[y][x] = game_map->grid[y][x];
            }
        }
    }
}