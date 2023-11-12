/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles player
 */

// View declaration.h for more details
#include <string.h>
#include <stdlib.h>
#include "message.h"
#include "player.h"
#include "declaration.h"

// static variable to assign unique IDs to players
static int playerID = 0;

// array of characters to assign IDs to players
char characters[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

// handle player/ spectator join
void handle_player_join(GameMap* game_map, addr_t from, char* player_name) {

    // check if the player already exists by matching the 'from' address
    Player* player = NULL;
    for (int i = 0; i < 26; i++) {
        if (game_map->players[i] && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];
            break;
        }
    }

    // if the player already exists, do nothing
    if (player != NULL) {
        return;
    }

    // case when spectator joins
    if (strcmp(player_name, "Spectator") == 0) {

        
        Player* player = malloc(sizeof(Player));

        if (!player) {
            perror("Error allocating memory for player");
            return; // Exit function since we can't do anything without memory
        }

        // initialize the spectator's attributes
        strcpy(player->name, player_name);
        player->from = from;
        player->gold_picked = 0;

        // if there was an existing spectator, inform them about replacement
        if (game_map->players[26] != NULL) {
            message_send(game_map->players[26]->from, "QUIT You have been replaced by a new spectator.");
        }

        // add the spectator player to the last slot in the players array
        game_map->players[26] = player;

        printf("Spectator joined\n");

        return;
    }

    // case when player joins
    if (playerID < 26) { // check if there is space for a new player
        Player* player = malloc(sizeof(Player));
        if (!player) {
            perror("Error allocating memory for player");
            return; // exit function since we can't do anything without memory
        }

        // initialize the new player's attributes
        strcpy(player->name, player_name);
        player->ID = characters[playerID]; 
        player->from = from; // Assign the address from which the player joined

        // Initialize other fields if necessary
        player->gold_count = 0;
        player->gold_picked = 0;

        // place the player on a random empty space on the map
        if (game_map->emptySpaceCount > 0) {
            int randomIndex = rand() % game_map->emptySpaceCount;

            player->position[0] = game_map->emptySpaces[randomIndex].x; 
            player->position[1] = game_map->emptySpaces[randomIndex].y;

            game_map->emptySpaces[randomIndex] = game_map->emptySpaces[game_map->emptySpaceCount - 1];
            game_map->emptySpaceCount--;
        }

        printf("Player %s joined with ID %c\n", player_name, player->ID);

        // update the game map with the new player
        game_map->players[playerID] = player;

        // allocate memory for the player's visible grid
        player->visible_grid = malloc(game_map->mapSizeC * sizeof(char*));
        for (int i = 0; i < game_map->mapSizeC; i++) {
            player->visible_grid[i] = malloc((game_map->mapSizeR + 1) * sizeof(char));
            memset(player->visible_grid[i], ' ', game_map->mapSizeR);
            player->visible_grid[i][game_map->mapSizeR] = '\0'; // Null-terminate the string
        }

        // check if player spawns on a gold pile, if so, collect it and spawn the player
        for (int i = 0; i < game_map->numGoldPiles; i++) {
            if (game_map->gold_piles[i].position[0] == player->position[0] &&
                game_map->gold_piles[i].position[1] == player->position[1]) {
                
                // player spawns on a gold pile, collect the gold
                player->gold_count += game_map->gold_piles[i].gold_count;
                game_map->goldLeft -= game_map->gold_piles[i].gold_count;

                // remove the gold pile
                if (i < game_map->numGoldPiles - 1) {
                    game_map->gold_piles[i] = game_map->gold_piles[game_map->numGoldPiles - 1];
                }
                game_map->numGoldPiles--;
                break; // gold pile found and processed, break out of the loop
            }
        }
        playerID++; // increment the playerID for the next player

    } else {
        // inform the player that the game is full
        message_send(from, "QUIT Game is full: no more players can join.\n");
    }
}

// function to handle collecting gold along the way
void collect_gold(Player* player, int newRow, int newCol, GameMap* game_map) {
    // if the new position is a gold pile, update the gold count and remove the pile
    for (int i = 0; i < game_map->numGoldPiles; i++) {
        if (game_map->gold_piles[i].position[0] == newCol && game_map->gold_piles[i].position[1] == newRow) {
            // increase player's gold count and the amount of gold picked up
            player->gold_count += game_map->gold_piles[i].gold_count;
            player->gold_picked += game_map->gold_piles[i].gold_count;

            // decrease the overall count of gold left in the game
            game_map->goldLeft -= game_map->gold_piles[i].gold_count;

            // remove the gold pile by swapping it with the last one in the array (if not already the last)
            if (i < game_map->numGoldPiles - 1) {
                game_map->gold_piles[i] = game_map->gold_piles[game_map->numGoldPiles - 1];
            }

            // decrease the count of gold piles
            game_map->numGoldPiles--;

            // print a message indicating that the player collected gold
            printf("Player %c collected gold at position (%d, %d)\n", player->ID, newCol, newRow);

            break; // exit the loop after handling the gold pile
        }
    }
}


// handles player's movement 
void handle_player_move(GameMap* game_map, addr_t from, char* moveDirectionStr) {
    // extract move direction from buf
    char moveDirection = moveDirectionStr[0];

    // find the player with the given ID
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

    // if the player is not found, return
    if (player == NULL) {
        fprintf(stderr, "Player not found for the given address.\n");
        return;
    }

    // calculate new position based on move direction
    int newRow = player->position[1];
    int newCol = player->position[0];

    // determine whether the movement should be continuous or not
    int step = (moveDirection >= 'A' && moveDirection <= 'Z') ? 1 : 0;

    // update player's position
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

    // check if the new position is within the bounds of the map and not occupied
    if (newRow >= 0 && newRow < game_map->mapSizeC && newCol >= 0 && newCol < game_map->mapSizeR &&
        (game_map->grid[newRow][newCol] == '.' || game_map->grid[newRow][newCol] == '#')) {

        // update player's position
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

        // collect gold at the new position
        collect_gold(player, newRow, newCol, game_map);
        
    } else {
        fprintf(stderr, "Invalid move. Position out of bounds or occupied.\n");
    }
}


// handles the quitting of a player or spectator
void handle_quit(GameMap* game_map, addr_t from) {
    // find the player with the given ID
    Player* player = NULL;
    for (int i = 0; i <= 26; i++) {
        if (game_map->players[i] != NULL && message_eqAddr(game_map->players[i]->from, from)) {
            player = game_map->players[i];

            // if the player is found, update their position to indicate they have quit
            if (player != NULL) {
                player->position[0] = -1;
                player->position[1] = -1;
            }

            // send a quit message to the player or spectator
            if (i == 26) {
                message_send(from, "QUIT Thanks for watching!");
            } else {
                message_send(from, "QUIT Thanks for playing!");
            }
            break;
        }
    }
}