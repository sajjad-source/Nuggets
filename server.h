/** server.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for server.h
*/

#include "struct.h"

/*==================== server.c ====================*/
/**************** handleMessage ****************/
/**
 * Caller provides:
 * 	valid argc, valid addr_t pointer, buffer
 *
 * TODO:
 * 	Initialize game
 *  Duplicate the message buffer to use with strtok
 *  Check if the duplication was successful
 *      Extract the command from the message
 *      Handle the "Join" command
 *      Handle the "Move" command
 *  Update and send the serialized map to all players
 *      Serialize the map with players and send it to each player
 *  Check if all gold is collected, and end the game if true
 *
 * RETURN:
 *  false to continue the message loop
 */
bool handleMessage(void *arg, const addr_t from, const char *buf);


/**************** game_over ****************/
/*
 * Caller provides:
 * 	valid GameMap pointer
 *
 * TODO:
 * 	Create a buffer to store the game over message
 *  Initialize the buffer
 *  Concatenate the initial game over message
 *  Iterate through players and append their information to the game over message
 *  Send the game over message to all players
 *
 */
void game_over(GameMap *game_map);
