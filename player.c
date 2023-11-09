/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles player
 */

// View declaration.h for more details

#include "support/message.h"

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
        
        player->visible_grid = malloc(game_map->mapSize * sizeof(char*));
        for (int i = 0; i < game_map->mapSize; i++) {
            player->visible_grid[i] = malloc((game_map->mapSize + 1) * sizeof(char));
            memset(player->visible_grid[i], ' ', game_map->mapSize);
            player->visible_grid[i][game_map->mapSize] = '\0'; // Null-terminate the string
        }
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