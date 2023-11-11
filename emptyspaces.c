/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles empty spaces on the map
 */

// View declaration.h for more details
Empty* find_empty_spaces(char** grid, int col, int row, int* count) {
    Empty* emptySpaces = malloc(row * col * sizeof(Empty));
    *count = 0; // Start with zero empty spaces found

    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {
            if (grid[i][j] == '.') {
                emptySpaces[*count].x = j;
                emptySpaces[*count].y = i;
                (*count)++;
            }
        }
    }

    // Shrink the array to the number of empty spaces found
    emptySpaces = realloc(emptySpaces, (*count) * sizeof(Empty));
    return emptySpaces;
}