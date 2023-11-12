/** gamemap.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for gamemap.c
*/

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "struct.h"

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

#endif // GAMEMAP_H