/** server.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support/message.h"
#include "struct.c"
#include "player.c"
#include "declaration.h"
#include "gamemap.c"
#include "gold.c"

// Declared the functions in declaration.h

int main(int argc, char *argv[])
{
    // Only 2 arguments should be provided
    if (argc != 2 && argc != 3)
    {
        printf("Usage: ./server <map_filename> [seed]\n");
        return 1;
    }

    // Initialize the messaging system and start listening on a port
    int port = message_init(stderr);
    if (port == 0)
    {
        fprintf(stderr, "Failed to initialize messaging system.\n");
        return 1;
    }

    // Load the game map and initialize the game state
    GameMap *game_map;
    if (argc == 3)
    {
        // Initialize with seed if provided
        game_map = initialize_game(argv[1], atoi(argv[2]));
    }
    else
    {
        // Initialize with default seed (10)
        game_map = initialize_game(argv[1], 10);
    }

    // handle error cases
    if (game_map == NULL)
    {
        fprintf(stderr, "Failed to initialize game.\n");
        return 1;
    }

    // Print the game map
    for (int i = 0; i < game_map->mapSize; i++)
    {
        printf("%s", game_map->grid[i]);
    }

    game_map->port = port; // Set port in the game map structure

    // Start the message loop, passing in handlers for different message types
    message_loop(game_map, 0, NULL, NULL, handleMessage);

    // Cleanup
    message_done();

    // free allocated memory
    free(game_map->emptySpaces);
    free(game_map->gold_piles);

    // Free player data
    for (int j = 0; j < 27; j++)
    {
        if (game_map->players[j] != NULL)
        {
            free(game_map->players[j]);
        }
    }

    // Free grid
    for (int j = 0; j < game_map->mapSize; j++)
    {
        free(game_map->grid[j]);
    }
    free(game_map->grid);

    // Free the game map structure
    free(game_map);

    return 0; // Exit successfully
}

bool handleMessage(void *arg, const addr_t from, const char *buf)
{

    GameMap *game_map = (GameMap *)arg; // cast argument to to gamemap
    char *message = strdup(buf);        // Duplicate the buffer to use with strtok

    // confirm that duplication was successful
    if (message != NULL)
    {
        char *command = strtok(message, " ");

        if (command != NULL && strcmp(command, "Join") == 0)
        {
            char *playerName = strtok(NULL, " ");
            if (playerName != NULL && strcmp(playerName, "Spectator") != 0)
            {
                handle_player_join(game_map, from, playerName);
            }
            else
            {
                handle_player_join(game_map, from, "Spectator");
            }
        }
        else
        {
            printf("Unknown command received.\n");
        }

        if (strcmp(command, "Move") == 0)
        {
            char *moveDirection = strtok(NULL, " ");
            if (moveDirection != NULL)
            {
                handle_player_move(game_map, from, moveDirection);
                // free(moveDirection);
            }
        }
        else
        {
            printf("Unknown command received.\n");
        }

        // free(command);

        printf("Message %s from: %s\n", buf, message_stringAddr(from));

        free(message); // Free the duplicated message buffer
    }

    for (int i = 0; i <= 26; i++)
    {
        if (game_map->players[i] != NULL)
        {
            // Serialize the map with players and send it to each player
            char *serializedMap = serialize_map_with_players(game_map, game_map->players[i]->from);
            if (serializedMap != NULL)
            {
                message_send(game_map->players[i]->from, serializedMap);
                free(serializedMap);
            }
        }
    }

    // Check if all gold is collected, and end the game if true
    if (game_map->goldLeft == 0)
    {
        game_over(game_map);
        return true;
    }

    return false; // Continue the message loop
}

void game_over(GameMap *game_map)
{

    // Create a buffer to store the game over message
    char gameOverMesg[1000];
    gameOverMesg[0] = '\0';

    strcat(gameOverMesg, "GAME OVER:\n"); // Concatenate the initial game over message

    // Iterate through players and append their information to the game over message
    for (int i = 0; i < 26; i++)
    {
        if (game_map->players[i] != NULL)
        {
            char message[100];
            // Format player information and concatenate to the game over message
            snprintf(message, 100, "%c %d %s\n", game_map->players[i]->ID, game_map->players[i]->gold_count, game_map->players[i]->name);
            strcat(gameOverMesg, message);
        }
    }

    // Send the game over message to all players
    for (int i = 0; i <= 26; i++)
    {
        if (game_map->players[i] != NULL)
        {
            message_send(game_map->players[i]->from, gameOverMesg);
        }
    }
}
