/** player.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles empty spaces on the map
 */

// View declaration.h for more details

Empty *find_empty_spaces(char **grid, int size, int *count)
{

    // Allocate memory for an array to store empty space coordinates
    Empty *emptySpaces = malloc(size * size * sizeof(Empty));

    // Initialize the count of empty spaces to zero
    *count = 0;

    // Iterate through each cell in the grid to find empty spaces
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {

            // Check if the current cell is an empty space ('.')
            if (grid[i][j] == '.')
            {

                // Store the coordinates of the empty space in the array
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