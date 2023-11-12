/** emptyspaces.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for emptyspaces.c
*/

#ifndef EMPTYSPACES_H
#define EMPTYSPACES_H

#include "struct.h"

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

#endif // EMPTYSPACES_H