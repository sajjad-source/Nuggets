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
int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        printf("Usage: ./server <map_filename> [seed]\n");
        return 1;
    }
    
    int port = message_init(stderr);
    if (port == 0) {
        fprintf(stderr, "Failed to initialize messaging system.\n");
        return 1;
    }

    int seed;
    if (argc == 3) {
        seed = atoi(argv[2]);
        if (seed <= 0) {
            fprintf(stderr, "Seed must be a positive integer.\n");
            return 1;
        }
    } else {
        seed = getpid();  // Use process ID as seed if not provided
    }

    GameMap* game_map = initialize_game(argv[1], seed);
    if (game_map == NULL) {
        fprintf(stderr, "Failed to initialize game.\n");
        return 1;
    }

    game_map->port = port;
    
    message_loop(game_map, 0, NULL, NULL, handleMessage);

    // Cleanup
    message_done();

    free(game_map->emptySpaces);

    free(game_map->gold_piles);
    
    for (int j = 0; j < 27; j++) {
        if (game_map->players[j] != NULL) {
            for (int i = 0; i < game_map->mapSizeC; i++) {
                free(game_map->players[j]->visible_grid[i]);
            }

            free(game_map->players[j]->visible_grid);
            free(game_map->players[j]);
        }
    }

    for (int j = 0; j < game_map->mapSizeC; j++) {
        free(game_map->grid[j]);
    }

    free(game_map->grid);
    free(game_map);
    
    return 0;
}

bool handleMessage(void* arg, const addr_t from, const char* buf) {
    GameMap* game_map = (GameMap*)arg;
    char* message = strdup(buf); // Duplicate the buffer to use with strtok

    if (message != NULL) {
        char* command = strtok(message, " ");

        if (command != NULL && strcmp(command, "PLAY") == 0) {
            char* playerName = strtok(NULL, " ");
            if (playerName != NULL && strcmp(playerName, "Spectator") != 0) {
                handle_player_join(game_map, from, playerName);
                // free(playerName);
            } else {
                handle_player_join(game_map, from, "Spectator");
            }
        } else if (strcmp(command, "KEY") == 0) {
            char* moveDirection = strtok(NULL, " ");
            if (moveDirection != NULL) {
            handle_player_move(game_map, from, moveDirection);
            }
        } else if (strcmp(command, "QUIT") == 0) {
            handle_quit(game_map, from);
        } else {
            printf("Unknown command received.\n");
        }

        printf("Message %s from: %s\n", buf, message_stringAddr(from));

        free(message); // Free the duplicated message buffer
    }


    for (int i = 0; i <= 26; i++) {
        if (game_map->players[i] != NULL) {
            // Use serialize_map_with_players to account for players on the map
            if (game_map->players[i]->position[0] != -1) {
                char* serializedMap = serialize_map_with_players(game_map, game_map->players[i]->from);
                if (serializedMap != NULL) {
                    message_send(game_map->players[i]->from, serializedMap);
                    free(serializedMap);
                }
            }
        }
    }

    if (game_map->goldLeft == 0) {
        game_over(game_map);
        return true;
    }

    return false; // Continue the message loop
}

void game_over(GameMap* game_map) {

    char gameOverMesg[10000];
    gameOverMesg[0] = '\0';

    strcat(gameOverMesg, "GAME OVER:\n");
    for (int i = 0; i < 26; i++) {
        if (game_map->players[i] != NULL) {
            char message[100];
            snprintf(message, 100, "%c %d %s\n", game_map->players[i]->ID, game_map->players[i]->gold_count, game_map->players[i]->name);
            strcat(gameOverMesg, message);
        }
    }
    for (int i = 0; i <= 26; i++) {
        if (game_map->players[i] != NULL) {
            message_send(game_map->players[i]->from, gameOverMesg);
        }
    }

}
