/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles player
 */

// View declaration.h for more details

#include "message.h"
#include "declaration.h"

void handle_player_join(GameMap* game_map, addr_t from, char* player_name) {

    Player* player = NULL;
    for (int i = 0; i < 26; i++) {
        if (game_map->players[i] && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];
            break;
        }
    }
    if (player != NULL) {
        return;
    }

    if (strcmp(player_name, "Spectator") == 0) {

        Player* player = malloc(sizeof(Player));

        if (!player) {
            perror("Error allocating memory for player");
            return; // Exit function since we can't do anything without memory
        }
        strcpy(player->name, player_name);

        player->from = from; // Assign the address from which the player joined

        if (game_map->players[26] != NULL) {
            message_send(game_map->players[26]->from, "QUIT You have been replaced by a new spectator.");
        }

        game_map->players[26] = player;

        printf("Spectator joined\n");

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
        
        player->visible_grid = malloc(game_map->mapSizeC * sizeof(char*));
        for (int i = 0; i < game_map->mapSizeC; i++) {
            player->visible_grid[i] = malloc((game_map->mapSizeR + 1) * sizeof(char));
            memset(player->visible_grid[i], ' ', game_map->mapSizeR);
            player->visible_grid[i][game_map->mapSizeR] = '\0'; // Null-terminate the string
        }

        // Check if player spawns on a gold pile, if so, collect it and spawn the player.
        for (int i = 0; i < game_map->numGoldPiles; i++) {
            if (game_map->gold_piles[i].position[0] == player->position[0] &&
                game_map->gold_piles[i].position[1] == player->position[1]) {
                
                // Player spawns on a gold pile, collect the gold
                player->gold_count += game_map->gold_piles[i].gold_count;
                game_map->goldLeft -= game_map->gold_piles[i].gold_count;

                // Remove the gold pile
                if (i < game_map->numGoldPiles - 1) {
                    game_map->gold_piles[i] = game_map->gold_piles[game_map->numGoldPiles - 1];
                }
                game_map->numGoldPiles--;
                break; // Gold pile found and processed, break out of the loop
            }
        }
        playerID++; // Increment the playerID for the next player

    } else {
        message_send(from, "QUIT Game is full: no more players can join.\n");
    }
}

// Function to handle collecting gold along the way
void collect_gold(Player* player, int newRow, int newCol, GameMap* game_map) {
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
}

void handle_player_move(GameMap* game_map, addr_t from, char* moveDirectionStr) {
    // Extract move direction from buf
    char moveDirection = moveDirectionStr[0];

    // Find the player with the given ID
    Player* player = NULL;
    for (int i = 0; i < 26; i++) {
        if (game_map->players[i] && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];
            if (player != NULL && player->position[0] == -1) {
                return;
            }
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

    // Determine whether the movement should be continuous or not
    int step = (moveDirection >= 'A' && moveDirection <= 'Z') ? 1 : 0;

    // Update player's position
    switch (moveDirection) {
        case 'h':
            newCol--;
            break; // Move left
        case 'H':
            while (newCol - step >= 0 && (game_map->grid[newRow][newCol - step] == '.' || game_map->grid[newRow][newCol - step] == '#')) {
                newCol -= step;
                player->position[0] = newCol;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
                
            }
            break; // Move left until player hits a wall
        case 'l':
            newCol++;
            break; // Move right
        case 'L':
            while (((newCol + step) < (game_map->mapSizeR)) && (game_map->grid[newRow][newCol + step] == '.' || game_map->grid[newRow][newCol + step] == '#')) {
                newCol += step;
                player->position[0] = newCol;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move right until player hits a wall
        case 'j':
            newRow++;
            break; // Move down
        case 'J':
            while (newRow + step < game_map->mapSizeC && (game_map->grid[newRow + step][newCol] == '.' || game_map->grid[newRow + step][newCol] == '#')) {
                newRow += step;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move down until player hits a wall
        case 'k':
            newRow--;
            break; // Move up
        case 'K':
            while (newRow - step >= 0 && (game_map->grid[newRow - step][newCol] == '.' || game_map->grid[newRow - step][newCol] == '#')) {
                newRow -= step;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move up until until player hits a wall
        case 'y':
            newRow--;
            newCol--;
            break; // Move diagonally up-left
        case 'Y':
            while (newRow - step >= 0 && newCol - step >= 0 && (game_map->grid[newRow - step][newCol - step] == '.' || game_map->grid[newRow - step][newCol - step] == '#')) {
                newRow -= step;
                newCol -= step;
                player->position[0] = newCol;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move diagonally up-left until player hits a wall

        case 'u':
            newRow--;
            newCol++;
            break; // Move diagonally up-right
        case 'U':
            while (newRow - step >= 0 && ((newCol + step) < (game_map->mapSizeR)) && (game_map->grid[newRow - step][newCol + step] == '.' || game_map->grid[newRow - step][newCol + step] == '#')) {
                newRow -= step;
                newCol += step;
                player->position[0] = newCol;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move diagonally up-right until player hits a wall
        case 'b':
            newRow++;
            newCol--;
            break; // Move diagonally down-left
        case 'B':
            while ((newRow + step < game_map->mapSizeC) && newCol - step >= 0 && (game_map->grid[newRow + step][newCol - step] == '.' || game_map->grid[newRow + step][newCol - step] == '#')) {
                newRow += step;
                newCol -= step;
                player->position[0] = newCol;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move diagonally down-left until player hits a wall
        case 'n':
            newRow++;
            newCol++;
            break; // Move diagonally down-right
         case 'N':
            while ((newRow + step < game_map->mapSizeC) && ((newCol + step) < (game_map->mapSizeR)) && (game_map->grid[newRow + step][newCol + step] == '.' || game_map->grid[newRow + step][newCol + step] == '#')) {
                newRow += step;
                newCol += step;
                player->position[0] = newCol;
                player->position[1] = newRow;
                calculate_visibility(game_map, player);
                collect_gold(player, newRow, newCol, game_map);
            }
            break; // Move diagonally down-right until player hits a wall

        default:
            fprintf(stderr, "Invalid move direction %c.\n", moveDirection);
            return;
    }

    // Check if the new position is within the bounds of the map and not occupied
    if (newRow >= 0 && newRow < game_map->mapSizeC && newCol >= 0 && newCol < game_map->mapSizeR &&
        (game_map->grid[newRow][newCol] == '.' || game_map->grid[newRow][newCol] == '#')) {

        // Update player's position
        Player* player2 = NULL;
        for (int i = 0; i < 26; i++) {
            if (game_map->players[i] != NULL && !message_eqAddr(game_map->players[i]->from, from)) {
                player2 = game_map->players[i];
                if (player2->position[1] == newRow && player2->position[0] == newCol) {
                    player2->position[0] = player->position[0];
                    player2->position[1] = player->position[1];
                }
                continue;
            }
        }
        player->position[1] = newRow;
        player->position[0] = newCol;

        collect_gold(player, newRow, newCol, game_map);
        
    } else {
        fprintf(stderr, "Invalid move. Position out of bounds or occupied.\n");
    }
}



void handle_quit(GameMap* game_map, addr_t from) {
    // Find the player with the given ID
    Player* player = NULL;
    for (int i = 0; i <= 26; i++) {
        if (game_map->players[i] != NULL && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];
            if (player != NULL) {
                player->position[0] = -1;
                player->position[1] = -1;
            }
            if (i == 26) {
                message_send(from, "QUIT Thanks for watching!");
            } else {
                message_send(from, "QUIT Thanks for playing!");
            }
            break;
        }
    }

}