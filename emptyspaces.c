/** 
 * emptyspaces.c
 * Team 6
 * Date: Nov 8, 2023
 * Description: This module is part of the 'Nugget' multiplayer game project, specifically dedicated to handling empty spaces within the 
 * game map. It includes functionality to scan and identify all the empty spaces in the game's grid. The file provides functions to locate 
 * these empty cells, store their coordinates, and manage the array of these coordinates. This management is vital for the game's logic, 
 * such as player movement, item distribution, and map analysis, ensuring a dynamic and interactive game environment. 
 */

#include <stdlib.h>
#include <string.h>
#include "emptyspaces.h"

/**************** find_empty_spaces() ****************/
/* see emptyspaces.h for description */
Empty* find_empty_spaces(char** grid, int col, int row, int* count) {

    // allocate memory for an array to store empty space coordinates
    Empty* emptySpaces = malloc(row * col * sizeof(Empty));
    *count = 0; // Start with zero empty spaces found

    // iterate through each cell in the grid
    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {

            // if the cell is empty, record its coordinates
            if (grid[i][j] == '.') {
                emptySpaces[*count].x = j;
                emptySpaces[*count].y = i;
                (*count)++;
            }
        }
    }

    // shrink the array to the number of empty spaces found
    emptySpaces = realloc(emptySpaces, (*count) * sizeof(Empty));
    return emptySpaces;
}