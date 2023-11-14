/** gamemap.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles the game map
*/

#define _POSIX_C_SOURCE 200809L 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "math.h"
#include "emptyspaces.h"
#include "gamemap.h"
#include "player.h"
#include "gold.h"

// View gamemap.h for more details

// initializes the game map based on the provided map file and seed.
GameMap *initialize_game(const char *map_filename, int seed)
{
    // Seed the random number generator
    if (seed == -1) {
        srand(getpid());
    } else {
        srand(seed);
    }

    // open the map file
    FILE *fp = fopen(map_filename, "r");
    if (fp == NULL)
    {
        perror("Error opening map file");
        return NULL;
    }

    // count the number of characters until a newline
    // int size = 0; // This will be the size for both rows and columns
    int row = 0;
    int col = 0;

    bool checkRow = true;

    char c;
    while ((c = fgetc(fp)) != EOF)
    {
        if (c != '\n' && checkRow) {
            row++;
        } else if (c == '\n') {
            checkRow = false;
            col++;
        }
    }

    // rewind to the start of the file to read the grid
    rewind(fp);

    // allocate memory for the GameMap structure
    GameMap *gameMap = malloc(sizeof(GameMap));
    if (gameMap == NULL)
    {
        perror("Error allocating memory for game map");
        fclose(fp);
        return NULL;
    }

    // set the size of the map
    gameMap->mapSizeR = row;
    gameMap->mapSizeC = col;

    // allocate memory for the grid
    gameMap->grid = malloc(gameMap->mapSizeC * sizeof(char*));
    if (gameMap->grid == NULL)
    {
        perror("Error allocating memory for grid");
        free(gameMap);
        fclose(fp);
        return NULL;
    }

    // allocate memory for each row in the grid
    for (int i = 0; i < col; i++)
    {
        gameMap->grid[i] = malloc((row + 2) * sizeof(char)); // +1 for new line and null terminator
        if (gameMap->grid[i] == NULL)
        {
            // handle error, free previously allocated memory
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

    // read the map from the file
    char lineBuffer[row + 2];                               // +2 for newline and null terminator
    for (int i = 0; i < col && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++)
    {
        if (i == col - 1)
        {
            strncpy(gameMap->grid[i], lineBuffer, row + 1);
            gameMap->grid[i][row] = '\0';                   // ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, row + 2);
        gameMap->grid[i][row] = '\n';                       // ensure null termination
        gameMap->grid[i][row + 1] = '\0';                   // ensure null termination
    }

    // find empty spaces after loading the map
    int emptySpaceCount;
    Empty *emptySpaces = find_empty_spaces(gameMap->grid, gameMap->mapSizeC, gameMap->mapSizeR, &emptySpaceCount);

    gameMap->emptySpaceCount = emptySpaceCount;
    gameMap->emptySpaces = emptySpaces;

    // check if there are no empty spaces on the map
    if (emptySpaceCount == 0)
    {
        fprintf(stderr, "No empty spaces found on the map.\n");
    }

    distribute_gold(gameMap, 250, 10, 30);

    fclose(fp); // close map

    return gameMap;
}

// serializes the game map with player information.
char* serialize_map_with_players(GameMap *gameMap, addr_t from) {
    // calculate buffer size: one char for each cell plus one for each newline, plus one for the null terminator
    int bufferSize = gameMap->mapSizeC * (gameMap->mapSizeR + 1) + 100;
    char* buffer = malloc(bufferSize);
    
    if (buffer == NULL) {
        perror("Error allocating buffer for serialization");
        return NULL;
    }

    // Create a copy of the map
    char** tempMap = malloc(gameMap->mapSizeC * sizeof(char*));
    for (int i = 0; i < gameMap->mapSizeC; i++) {
        tempMap[i] = strdup(gameMap->grid[i]);                                      // Copy each row
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
            calculate_visibility(gameMap, current_player);                          // Calculate visibility for the current player
            break;
        }
    }

    char* playerInfo = malloc(bufferSize);                                          // Assuming this is enough space for the player info

    // Overlay players' positions on the temporary map
    for (int i = 0; i < 26; i++) {
        Player* player = gameMap->players[i];
        if (player != NULL) {
            // Ensure the position is within the bounds of the map
            if (player->position[0] >= 0 && player->position[0] < gameMap->mapSizeR &&
                player->position[1] >= 0 && player->position[1] < gameMap->mapSizeC) {
                if (!message_eqAddr(player->from, from)) {
                    tempMap[player->position[1]][player->position[0]] = player->ID; // Use the player's ID as the character
                } else {
                    tempMap[player->position[1]][player->position[0]] = '@';        // Use the player's ID as the character
                    if (player->gold_picked > 0) {
                        snprintf(playerInfo, bufferSize, "Player %c has %d nuggets (%d nuggets unclaimed). Gold received: %d\n",
                        player->ID, player->gold_count, gameMap->goldLeft, player->gold_picked);
                        player->gold_picked = 0;
                    } else {
                        snprintf(playerInfo, bufferSize, "Player %c has %d nuggets (%d nuggets unclaimed).\n",
                        player->ID, player->gold_count, gameMap->goldLeft);
                    }
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
        if (goldPile.position[0] >= 0 && goldPile.position[0] < gameMap->mapSizeR &&
            goldPile.position[1] >= 0 && goldPile.position[1] < gameMap->mapSizeC)
        {
            if (gameMap->players[26] != NULL && message_eqAddr(gameMap->players[26]->from, from)) {
                tempMap[goldPile.position[1]][goldPile.position[0]] = '*';
            } else if (line_of_sight(gameMap, current_player->position[0], current_player->position[1], goldPile.position[0], goldPile.position[1])) {
                tempMap[goldPile.position[1]][goldPile.position[0]] = '*';
            }
        }
    }

    char *p = buffer;

    if (gameMap->players[26] != NULL && message_eqAddr(from, gameMap->players[26]->from)) {
        for (int i = 0; i < gameMap->mapSizeC; i++) {
            strncpy(p, tempMap[i], gameMap->mapSizeR + 1);                          // Copy each row including the newline
            p += gameMap->mapSizeR + 1;                                             // Move the pointer by the size of the row plus newline
        }
        *p = '\0';                                                                  // Null-terminate the buffer
    } else {
        for (int y = 0; y < gameMap->mapSizeC; y++)
        {
            for (int x = 0; x < gameMap->mapSizeR; x++)
            {
                if (current_player != NULL && current_player->visible_grid[y][x] != ' ')
                {
                    p[x] = tempMap[y][x];                                           // Place visible character from tempMap
                }
                else
                {
                p[x] = ' ';                                                         // If not visible, place a space
                }
            }   
            p += gameMap->mapSizeR;                                                 // Move the pointer by the size of the row
            *p = '\n';                                                              // Add newline at the end of each row
            p++;                                                                    // Move past the newline
        }
        *p = '\0';                                                                  // Null-terminate the buffer
    }


    // Free the temporary map
    for (int i = 0; i < gameMap->mapSizeC; i++) {
        free(tempMap[i]);
    }
    free(tempMap);

    strcat(playerInfo, buffer);

    free(buffer);

    return playerInfo;                                                              // Return the serialized buffer
}

// Determine if there is clear path using Bresenham's Line Algorithm
bool is_clear_path(GameMap *game_map, int start_x, int start_y, int end_x, int end_y)
{
    float delta_x = abs(end_x - start_x);
    float delta_y = -abs(end_y - start_y);
    float step_x = start_x < end_x ? 1 : -1;
    float step_y = start_y < end_y ? 1 : -1;
    float error = delta_x + delta_y;                                                // The error accumulator

    while (true)
    {
        // Check if we've reached the target point
        if (start_x == end_x && start_y == end_y)
        {
            // Check if the current position is a wall
            char current_pos = game_map->grid[start_y][start_x];
            if (current_pos == '-' || current_pos == '|' || current_pos == '+')
            {
                return true;                                                        // The wall is visible
            }
            break;                                                                  // Reached the target, end the loop
        }

        // Check if the current position is a clear path
        if (game_map->grid[start_y][start_x] != '.' && game_map->grid[start_y][start_x] != ' ')
        {
            return false;                                                           // It's not a clear path
        }

        int error2 = 2 * error;
        if (error2 >= delta_y)
        {
            error += delta_y;                                                       // Adjust the error
            start_x += step_x;                                                      // Take a step in x
        }
        if (error2 <= delta_x)
        {
            error += delta_x;                                                       // Adjust the error
            start_y += step_y;                                                      // Take a step in y
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
    for (int y = 0; y < game_map->mapSizeC; y++)
    {
        for (int x = 0; x < game_map->mapSizeR; x++)
        {
            bool visible = false;
            char currentCell = game_map->grid[y][x];
            // For walls, corners, and non-wall tiles, use original logic
            if (currentCell == '-' || currentCell == '|' || currentCell == '+')
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        int checkX = x + dx;
                        int checkY = y + dy;
                        // Ensure that we don't check out of bounds or through corners
                        if (checkY >= 0 && checkY < game_map->mapSizeC && checkX >= 0 && checkX < game_map->mapSizeR)
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
            }
            else if (line_of_sight(game_map, player->position[0], player->position[1], x, y))
            {
                visible = true;
            }
            // Additional check to ensure no adjacent corners are visible
            if (visible && currentCell == '+')
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        // Skip checking the current cell
                        if (dx == 0 && dy == 0) continue;
                        int adjacentX = x + dx;
                        int adjacentY = y + dy;
                        // Check for out of bounds
                        if (adjacentY >= 0 && adjacentY < game_map->mapSizeC && adjacentX >= 0 && adjacentX < game_map->mapSizeR)
                        {
                
                            // Check for adjacent corner visibility
                            if ((game_map->grid[adjacentY][adjacentX] == '-') && line_of_sight(game_map, player->position[0], player->position[1], adjacentX, adjacentY))
                            {
                                visible = true; // Make current corner invisible
                                // break;
                            }
                            // Check for adjacent corner visibility
                            if ((game_map->grid[adjacentY][adjacentX] == '+') && line_of_sight(game_map, player->position[0], player->position[1], adjacentX, adjacentY))
                            {
                                visible = false; // Make current corner invisible
                                // break;
                            }
                        }
                    }
                    if (!visible) break;
                }
            }
            // Additional check to ensure no adjacent corners are visible
            if (visible && currentCell == '-')
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = 0; dx <= 0; dx++)
                    {
                        // Skip checking the current cell
                        if (dx == 0 && dy == 0) continue;
                        int adjacentX = x + dx;
                        int adjacentY = y + dy;
                        // Check for out of bounds
                        if (adjacentY >= 0 && adjacentY < game_map->mapSizeC && adjacentX >= 0 && adjacentX < game_map->mapSizeR)
                        {
                            // Check for adjacent corner visibility
                            if ((game_map->grid[adjacentY][adjacentX] == '-' || game_map->grid[adjacentY][adjacentX] == '+' ||  game_map->grid[adjacentY][adjacentX] == '|') && line_of_sight(game_map, player->position[0], player->position[1], adjacentX, adjacentY))
                            {
                                visible = false; // Make current corner invisible
                                break;
                            }
                        }
                    }
                    if (!visible) break;
                }
            }
            if (visible && currentCell == '+')
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = 0; dx <= 0; dx++)
                    {
                        // Skip checking the current cell
                        if (dx == 0 && dy == 0) continue;
                        int adjacentX = x + dx;
                        int adjacentY = y + dy;
                        // Check for out of bounds
                        if (adjacentY >= 0 && adjacentY < game_map->mapSizeC && adjacentX >= 0 && adjacentX < game_map->mapSizeR)
                        {
                            // Check for adjacent corner visibility
                            if ((game_map->grid[adjacentY][adjacentX] == '+') && line_of_sight(game_map, player->position[0], player->position[1], adjacentX, adjacentY))
                            {
                                visible = false; // Make current corner invisible
                                break;
                            }
                        }
                    }
                    if (!visible) break;
                }
            }
            // Additional check to ensure no adjacent corners are visible
            if (visible && currentCell == '|')
            {
                for (int dy = 0; dy <= 0; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        // Skip checking the current cell
                        if (dx == 0 && dy == 0) continue;
                        int adjacentX = x + dx;
                        int adjacentY = y + dy;
                        // Check for out of bounds
                        if (adjacentY >= 0 && adjacentY < game_map->mapSizeC && adjacentX >= 0 && adjacentX < game_map->mapSizeR)
                        {
                            // Check for adjacent corner visibility
                            if ((game_map->grid[adjacentY][adjacentX] == '|' || game_map->grid[adjacentY][adjacentX] == '-' || game_map->grid[adjacentY][adjacentX] == '+') && line_of_sight(game_map, player->position[0], player->position[1], adjacentX, adjacentY))
                            {
                                visible = false; // Make current corner invisible
                                break;
                            }
                        }
                    }
                    if (!visible) break;
                }
            }
            // Additional check to ensure no adjacent corners are visible
            if (visible && currentCell == '+') {
                for (int dy = -1; dy <= 1; dy += 2) {  // Check only above and below
                    int adjacentY = y + dy;

                    // Check for out of bounds
                    if (adjacentY >= 0 && adjacentY < game_map->mapSizeC) {
                        char adjacentCell = game_map->grid[adjacentY][x];

                        // Check for adjacent ceiling or floor
                        if (adjacentCell == '-') {
                            visible = false; // Make current corner invisible
                            break;
                        }
                    }
                }
            }
            if (currentCell == '+' && (x-1 >= 0 && x-1 < game_map->mapSizeR) && player->visible_grid[y][x-1] == '-') {
                visible = true;
            }
            if (currentCell == '+' && (x+1 >= 0 && x+1 < game_map->mapSizeR) && player->visible_grid[y][x+1] == '-') {
                visible = true;
            }
            if (currentCell == '+' && player->visible_grid[player->position[1]][player->position[0]] == '#') {
                visible = false;
            }
            if (currentCell == '-' && player->visible_grid[player->position[1]][player->position[0]] == '#') {
                visible = false;
            }
            if (currentCell == '|' && player->visible_grid[player->position[1]][player->position[0]] == '#') {
                visible = false;
            }

            if (visible)
            {
        
                player->visible_grid[y][x] = currentCell;
            }

        }
        
    }

}