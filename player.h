

#include "struct.h"
#include "support/message.h"

/**************** handle_player_join ****************/
/**
 * Caller provides:
 * 	valid GameMap pointer, valid addr_t pointer and buffer
 *
 * TODO:
 *  Check if the player already exists by matching the 'from' address
 *  If the player already exists, do nothing
 * 
 *  Case when a spectator joins
 *      Allocate memory for a new spectator player
 *      Initialize the spectator's attributes
 *      If there was an existing spectator, inform them about replacement
 *      Add the spectator player to the last slot in the players array
 * 
 *  Case when a player joins
 *      Check if there is space for a new player
 *      Allocate memory for a new player
 *      Initialize the new player's attributes
 *      Place the player on a random empty space on the map
 *      Update the game map with the new player
 *      Allocate memory for the player's visible grid
 *      Check if player spawns on a gold pile, if so, collect it and spawn the player
 *      Increment the playerID for the next player
 *  Inform the player that the game is full
 */
void handle_player_join(GameMap *game_map, addr_t from, char *buf);


/**************** collect_gold ****************/
/**
 * Caller provides:
 * 	valid Player pointer,valid row, valid column and GameMap pointer
 *
 * TODO:
 *  If the new position is a gold pile, update the gold count and remove the pile
 *  Check if the new position matches the position of a gold pile
 *      Increase player's gold count and the amount of gold picked up
 *      Decrease the overall count of gold left in the game
 *      Remove the gold pile by swapping it with the last one in the array (if not already the last)
 *  Decrease the count of gold piles on the map
 */
void collect_gold(Player* player, int newRow, int newCol, GameMap* game_map);


/**************** handle_player_move ****************/
/**
 * Function to handle player movement
 * 
 * Caller provides:
 * 	valid GameMap pointer, addr_t pointer and buffer
 *
 * TODO:
 *  Extract move direction from the buffer
 *  Find the player with the given ID
 *  If the player is not found, print an error and return
 *  Calculate new position based on move direction
 *  Determine whether the movement should be continuous or not
 *  Update player's position while collecting gold
 *      h - left, l - right, j - down, k - up
 *      y - diagonally up-left u - diagonally up-right, 
 *      b - diagonally down-left n - diagonally down-right
 *  Check if the new position is within the bounds of the map and not occupied
 *       Update player's position
 *       Collect gold at the new position
 *  else: Invalid move
 *
 */
void handle_player_move(GameMap *game_map, addr_t from, char *moveDirectionStr);


/**************** handle_quit ****************/
/** 
 * Function is used to find empty spaces on the map
 * 
 * Caller provides:
 * 	valid GameMap pointer and addr_t pointer
 *
 * TODO:
 *  find the player with the given ID
 *  if the player is found, update their position to indicate they have quit
 *  send the quit message to player / spectator
 * 
 */
void handle_quit(GameMap* game_map, addr_t from);
