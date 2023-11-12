/** declaration.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: Bash script that declares all the functions to be used by server.c, player.c,
 *          gold.c, gamemap.c
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



/*==================== emptyspaces.c ====================*/
/**************** find_empty_spaces ****************/
/** 
 * Function is used to find empty spaces on the map
 * 
 * Caller provides:
 * 	valid grid, size and count
 *
 * TODO:
 *  Allocate memory for an array to store empty space coordinates
 *  Initialize the count of empty spaces to zero
 *  Iterate through each cell in the grid to find empty spaces
 *      Check if the current cell is an empty space ('.')
 *           Store the coordinates of the empty space in the array
 *           Increment the count of empty spaces
 *      Shrink the array to the number of empty spaces found
 * 
 *  RETURN:
 *      Array of empty space coordinates
 */
Empty* find_empty_spaces(char** grid, int row, int col, int* count);


/*==================== gold.c ====================*/
/**************** distribute gold ****************/
/** 
 * Function is used to distribute gold pile on the map
 * 
 * Caller provides:
 * 	valid GameMap pointer, total gold, minimum gold piles and max gold piles
 *
 * TODO:
 *  Calculate map area
 *  Define a scaling factor based on map size (e.g., mapArea / 100)
 *  This factor will help determine the number of gold piles relative to map size
 *  Calculate a proportional number of gold piles based on map size
 *  Determine the final number of gold piles, ensuring it doesn't exceed empty spaces
 *  Allocate memory for the gold piles
 *  Distribute gold among the piles: Ensure each pile gets at least 1 gold piece
 *  Distribute the remaining gold
 *  Shuffle the empty spaces to randomize gold pile placement
 *  Place gold piles on the map
 *  Update the game map's gold-related attributes
 */
void distribute_gold(GameMap *game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles);


/*==================== gamemap.c ====================*/
/**************** initialize_game ****************/
/**
 * Caller provides:
 * 	valid map filename and seed
 *
 * TODO:
 *  Seed the random number generator
 *  Open the map file
 *  Count the number of characters until a newline
 *  Rewind to the start of the file to read the grid
 *  Allocate memory for the GameMap structure
 *  Set the size of the map
 *  Allocate memory for the grid
 *  Allocate memory for each row in the grid
 * 
 *   Read the map from the file
 *   Find empty spaces after loading the map
 *   Check if there are no empty spaces on the map
 *   distribute the gold
 * 
 * RETURN:
 *  gamemap
 */
GameMap *initialize_game(const char *map_filename, int seed);


/**************** serialize_map_with_players ****************/
/**
 * Caller provides:
 * 	valid GameMap pointer and addr_t pointer
 *
 * TODO:
 *  Calculate buffer size: one char for each cell plus one for each newline, 
 *  plus one for the null terminator
 *  Create a copy of the map
 *      Handle allocation failure
 *  Overlay players' positions on the temporary map
 *  Overlay gold piles positions on the temporary map
 *  Serialize the temporary map into the buffer
 *  Free the temporary map
 * 
 * RETURN:
 *  serialized buffer [playerInfo]
 *
 */
char *serialize_map_with_players(GameMap *gameMap, addr_t from);


/*==================== visibility ====================*/
/**************** is_clear_path ****************/
/**
 * Checks if there is a clear path from the starting point to the ending point on the game map.
 * Uses Bresenham's line algorithm to determine visibility.
 * 
 * Caller provides:
 * 	valid GameMap pointer, start x, start y, end x, end y
 *
 * TODO:
 *  Check if we've reached the target point
 *      Check if the current position is a wall
 *      If the wall is visible return true
 *      else break (reached the target)
 *  Check if the current position is a clear path
 *      return false (not clear)
 * 
 * RETURN:
 *  true (clear path)
 */
bool is_clear_path(GameMap* game_map, int start_x, int start_y, int end_x, int end_y);


/**************** line_of_sight ****************/
/**
 * Checks if there is a line of sight between two points on the game map.
 * 
 * Caller provides:
 * 	valid GameMap pointer, player row, player column, target row, target column
 *
 * TODO:
 *  If directly adjacent spots are visible, return true
 *  Check if there is a clear path to the target spot
 * 
 * RETURN:
 *  true if adjacent spots are visible
 */
bool line_of_sight(GameMap *game_map, int player_row, int player_col, int target_row, int target_col);


/**************** calculate_visibility ****************/
/**
 * Calculates the visibility for a player on the game map.
 * 
 * Caller provides:
 * 	valid GameMap pointer, and player
 *
 * TODO:
 *  Create a visible grid for the player with the same size as the game_map
 *  Check visibility for every cell in the grid
 *      For walls, check adjacent tiles as well as direct line of sight
 *      Ensure that we don't check out of bounds or through corners
 *          Diagonal checks are through corners, so we skip them
 *          Break out of the inner loop on first visible
 *      Break out of the outer loop if visible
 *  For non-wall tiles, use direct line of sight check
 */
void calculate_visibility(GameMap* game_map, Player* player);

