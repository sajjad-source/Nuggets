/** struct.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for all the data structures used
*/

#ifndef STRUCT_H
#define STRUCT_H

#include "support/message.h"


// Player structure
typedef struct Player {
    char ID;             // unique identifier for the player
    char name[50];       // name of the player (up to 50 characters)
    int position[2];     // current position of the player on the game map (x, y)
    int gold_count;      // total gold count collected by the player
    int gold_picked;     // gold count picked up during the last move
    char **visible_grid; // 2D array representing the visible portion of the game grid for the player
    addr_t from;         // address information for the player
} Player;

// GoldPile structure
typedef struct
{
    int position[2]; // position of the gold pile on the game map (x, y)
    int gold_count;  // gold count in the gold pile
} GoldPile;

// Empty structure
typedef struct
{
    int x; // x-coordinate of an empty space
    int y; // y-coordinate of an empty space
} Empty;

// GameMap structure
typedef struct
{
    char **grid;          // 2D array representing the game map
    GoldPile *gold_piles; // array of gold piles on the map
    int emptySpaceCount;  // number of empty spaces on the map
    Empty *emptySpaces;   // array of empty spaces on the map
    Player *players[27];  // array to store player information (up to 26 players + 1 spectator)
    int mapSizeR;         // number of rows in the game map
    int mapSizeC;         // number of columns in the game map
    int numGoldPiles;     // number of gold piles on the map
    int goldLeft;         // total amount of gold left on the map
    int port;             // port information for the game
} GameMap;

#endif // STRUCT_H