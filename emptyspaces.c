
// This function is used to find empty spaces on the map
Empty* find_empty_spaces(char** grid, int size, int* count) {
    Empty* emptySpaces = malloc(size * size * sizeof(Empty));
    *count = 0; // Start with zero empty spaces found

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
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