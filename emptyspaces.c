/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles empty spaces on the map
 */

// View declaration.h for more details

// finds and returns the coordinates of empty spaces in the grid
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