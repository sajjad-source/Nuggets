/** gold.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for gold.c
*/

#include "struct.h"

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
