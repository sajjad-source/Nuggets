/** gamemap.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles the game map
 */

// View declaration.h for more details
#include "math.h"
#include "emptyspaces.c"
#include <unistd.h>

GameMap *initialize_game(const char *map_filename, int seed)
{

    // Seed the random number generator
    if (seed == -1) {
        srand(getpid());
    } else {
        srand(seed);
    }
    

    // Open the map file
    FILE *fp = fopen(map_filename, "r");
    if (fp == NULL)
    {
        perror("Error opening map file");
        return NULL;
    }

    // Assuming the map is always square, count the number of characters until a newline
    int size = 0; // This will be the size for both rows and columns
    while (fgetc(fp) != '\n')
    {
        size++;
    }

    // Rewind to the start of the file to read the grid
    rewind(fp);

    // Allocate memory for the GameMap structure
    GameMap *gameMap = malloc(sizeof(GameMap));
    if (gameMap == NULL)
    {
        perror("Error allocating memory for game map");
        fclose(fp);
        return NULL;
    }

    gameMap->mapSize = size;

    // Allocate memory for the grid
    gameMap->grid = malloc(size * sizeof(char *));
    if (gameMap->grid == NULL)
    {
        perror("Error allocating memory for grid");
        free(gameMap);
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < size; i++)
    {
        gameMap->grid[i] = malloc((size + 2) * sizeof(char)); // +1 for new line and null terminator
        if (gameMap->grid[i] == NULL)
        {
            // Handle error, free previously allocated memory
            for (int j = 0; j < i; j++)
            {
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
    for (int i = 0; i < size && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++)
    {
        if (i == size - 1)
        {
            strncpy(gameMap->grid[i], lineBuffer, size + 1);
            gameMap->grid[i][size] = '\0'; // Ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, size + 2);
        gameMap->grid[i][size] = '\n';     // Ensure null termination
        gameMap->grid[i][size + 1] = '\0'; // Ensure null termination
    }

    // Find empty spaces after loading the map
    int emptySpaceCount;
    Empty *emptySpaces = find_empty_spaces(gameMap->grid, gameMap->mapSize, &emptySpaceCount);

    gameMap->emptySpaceCount = emptySpaceCount;

    gameMap->emptySpaces = emptySpaces;

    if (emptySpaceCount == 0)
    {
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

    Player *current_player = NULL;
    for (int i = 0; i < 26; i++)
    {
        if (gameMap->players[i] != NULL && message_eqAddr(gameMap->players[i]->from, from))
        {
            current_player = gameMap->players[i];
            calculate_visibility(gameMap, current_player); // Calculate visibility for the current player
            // break;
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
    for (int i = 0; i < gameMap->numGoldPiles; i++)
    { // Assuming numGoldPiles is the number of gold piles
        GoldPile goldPile = gameMap->gold_piles[i];
        // Ensure the position is within the bounds of the map
        if (goldPile.position[0] >= 0 && goldPile.position[0] < gameMap->mapSize &&
            goldPile.position[1] >= 0 && goldPile.position[1] < gameMap->mapSize)
        {
            tempMap[goldPile.position[1]][goldPile.position[0]] = '*';
        }
    }

    char *p = buffer;

    if (gameMap->players[26] != NULL && message_eqAddr(from, gameMap->players[26]->from)) {
        for (int i = 0; i < gameMap->mapSize; i++) {
            strncpy(p, tempMap[i], gameMap->mapSize + 1); // Copy each row including the newline
            p += gameMap->mapSize + 1; // Move the pointer by the size of the row plus newline
        }
        *p = '\0'; // Null-terminate the buffer
    } else {
        for (int y = 0; y < gameMap->mapSize; y++)
        {
            for (int x = 0; x < gameMap->mapSize; x++)
            {
                if (current_player != NULL && current_player->visible_grid[y][x] != ' ')
                {
                    p[x] = tempMap[y][x]; // Place visible character from tempMap
                }
                else
                {
                p[x] = ' '; // If not visible, place a space
                }
            }   
            p += gameMap->mapSize; // Move the pointer by the size of the row
            *p = '\n';             // Add newline at the end of each row
            p++;                   // Move past the newline
        }
        *p = '\0'; // Null-terminate the buffer
    }


    // Free the temporary map
    for (int i = 0; i < gameMap->mapSize; i++) {
        free(tempMap[i]);
    }
    free(tempMap);

    strcat(playerInfo, buffer);

    free(buffer);

    return playerInfo; // Return the serialized buffer
}

// Visibility
bool is_clear_path(GameMap *game_map, int start_x, int start_y, int end_x, int end_y)
{
    float delta_x = abs(end_x - start_x);
    float delta_y = -abs(end_y - start_y);
    float step_x = start_x < end_x ? 1 : -1;
    float step_y = start_y < end_y ? 1 : -1;
    float error = delta_x + delta_y; // The error accumulator

    while (true)
    {
        // Check if we've reached the target point
        if (start_x == end_x && start_y == end_y)
        {
            // Check if the current position is a wall
            char current_pos = game_map->grid[start_y][start_x];
            if (current_pos == '-' || current_pos == '|' || current_pos == '+')
            {
                return true; // The wall is visible
            }
            break; // Reached the target, end the loop
        }

        // Check if the current position is a clear path
        if (game_map->grid[start_y][start_x] != '.' && game_map->grid[start_y][start_x] != ' ')
        {
            return false; // It's not a clear path
        }

        int error2 = 2 * error;
        if (error2 >= delta_y)
        {
            error += delta_y;  // Adjust the error
            start_x += step_x; // Take a step in x
        }
        if (error2 <= delta_x)
        {
            error += delta_x;  // Adjust the error
            start_y += step_y; // Take a step in y
        }
    }

    // Clear path if we've reached this point
    return true;
}

bool line_of_sight(GameMap *game_map, int player_row, int player_col, int target_row, int target_col)
{
    // Directly adjacent spots are always visible
    if (abs(player_row - target_row) <= 1 && abs(player_col - target_col) <= 1)
    {
        return true;
    }

    // Check if there is a clear path to the target spot
    return is_clear_path(game_map, player_row, player_col, target_row, target_col);
}

void calculate_visibility(GameMap *game_map, Player *player)
{
    // Check visibility for every cell in the grid
    for (int y = 0; y < game_map->mapSize; y++)
    {
        for (int x = 0; x < game_map->mapSize; x++)
        {
            // For walls, check adjacent tiles as well as direct line of sight
            if (game_map->grid[y][x] == '-' || game_map->grid[y][x] == '|' || game_map->grid[y][x] == '+')
            {
                bool visible = false;
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        int checkX = x + dx;
                        int checkY = y + dy;
                        // Ensure that we don't check out of bounds or through corners
                        if (checkY >= 0 && checkY < game_map->mapSize && checkX >= 0 && checkX < game_map->mapSize)
                        {
                            // Diagonal checks are through corners, so we skip them
                            if (dx != 0 && dy != 0)
                                continue;
                            if (line_of_sight(game_map, player->position[0], player->position[1], checkX, checkY))
                            {
                                visible = true;
                                break; // Break out of the inner loop on first visible
                            }
                        }
                    }
                    if (visible)
                        break; // Break out of the outer loop if visible
                }
                if (visible)
                {
                    player->visible_grid[y][x] = game_map->grid[y][x];
                }
            }
            // For non-wall tiles, use direct line of sight check
            else if (line_of_sight(game_map, player->position[0], player->position[1], x, y))
            {
                player->visible_grid[y][x] = game_map->grid[y][x];
            }
        }
    }
}