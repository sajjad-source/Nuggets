#include "emptyspaces.c"
GameMap* initialize_game(const char* map_filename, int seed) {

    // Seed the random number generator
    srand(seed);

    // Open the map file
    FILE* fp = fopen(map_filename, "r");
    if (fp == NULL) {
        perror("Error opening map file");
        return NULL;
    }

    // Assuming the map is always square, count the number of characters until a newline
    int size = 0; // This will be the size for both rows and columns
    while (fgetc(fp) != '\n') {
        size++;
    }

    // Rewind to the start of the file to read the grid
    rewind(fp);

    // Allocate memory for the GameMap structure
    GameMap* gameMap = malloc(sizeof(GameMap));
    if (gameMap == NULL) {
        perror("Error allocating memory for game map");
        fclose(fp);
        return NULL;
    }

    gameMap->mapSize = size;

    // Allocate memory for the grid
    gameMap->grid = malloc(size * sizeof(char*));
    if (gameMap->grid == NULL) {
        perror("Error allocating memory for grid");
        free(gameMap);
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        gameMap->grid[i] = malloc((size + 2) * sizeof(char)); // +1 for new line and null terminator
        if (gameMap->grid[i] == NULL) {
            // Handle error, free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(gameMap->grid[j]);
            }
            free(gameMap->grid);
            free(gameMap);
            fclose(fp);
            return NULL;
        }
    }

    // Read the map from the file
    char lineBuffer[size + 2];              // +2 for newline and null terminator
    for (int i = 0; i < size && fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL; i++) {
        if (i == size-1) {
            strncpy(gameMap->grid[i], lineBuffer, size+1);
            gameMap->grid[i][size] = '\0';  // Ensure null termination
            break;
        }
        strncpy(gameMap->grid[i], lineBuffer, size+2);
        gameMap->grid[i][size] = '\n';      // Ensure null termination
        gameMap->grid[i][size+1] = '\0';    // Ensure null termination
    }

    // Find empty spaces after loading the map
    int emptySpaceCount;
    Empty* emptySpaces = find_empty_spaces(gameMap->grid, gameMap->mapSize, &emptySpaceCount);

    gameMap->emptySpaceCount = emptySpaceCount;

    gameMap->emptySpaces = emptySpaces;

    if (emptySpaceCount == 0) {
        fprintf(stderr, "No empty spaces found on the map.\n");
    }

    distribute_gold(gameMap, 250, 10, 30);

    fclose(fp);

    return gameMap;
}

char* serialize_map_with_players(GameMap *gameMap, addr_t from) {
    // Calculate buffer size: one char for each cell plus one for each newline, plus one for the null terminator
    int bufferSize = gameMap->mapSize * (gameMap->mapSize + 1) + 100;
    char* buffer = malloc(bufferSize);
    
    if (buffer == NULL) {
        perror("Error allocating buffer for serialization");
        return NULL;
    }

    // Create a copy of the map
    char** tempMap = malloc(gameMap->mapSize * sizeof(char*));
    for (int i = 0; i < gameMap->mapSize; i++) {
        tempMap[i] = strdup(gameMap->grid[i]); // Copy each row
        if (tempMap[i] == NULL) {
            // Handle allocation failure; free previously allocated strings and the array, then return NULL
            for (int j = 0; j < i; j++) {
                free(tempMap[j]);
            }
            free(tempMap);
            free(buffer);
            return NULL;
        }
    }

    char* playerInfo = malloc(bufferSize); // Assuming this is enough space for the player info

    // Overlay players' positions on the temporary map
    for (int i = 0; i < 26; i++) {
        Player* player = gameMap->players[i];
        if (player != NULL) {
            // Ensure the position is within the bounds of the map
            if (player->position[0] >= 0 && player->position[0] < gameMap->mapSize &&
                player->position[1] >= 0 && player->position[1] < gameMap->mapSize) {
                if (!message_eqAddr(player->from, from)) {
                    tempMap[player->position[1]][player->position[0]] = player->ID; // Use the player's ID as the character
                } else {
                    tempMap[player->position[1]][player->position[0]] = '@'; // Use the player's ID as the character
                    snprintf(playerInfo, bufferSize, "Player %c has %d nuggets (%d nuggets unclaimed).\n",
                     player->ID, player->gold_count, gameMap->goldLeft);
                }
            }
        }
    }

    if (gameMap->players[26] != NULL) {
        if (message_eqAddr(from, gameMap->players[26]->from)) {
            snprintf(playerInfo, bufferSize, "Spectator: %d nuggets unclaimed. Play at plank %d\n", gameMap->goldLeft, gameMap->port);
        }
    }

    // Overlay gold piles positions on the temporary map
    for (int i = 0; i < gameMap->numGoldPiles; i++) { // Assuming numGoldPiles is the number of gold piles
        GoldPile goldPile = gameMap->gold_piles[i];
        // Ensure the position is within the bounds of the map
        if (goldPile.position[0] >= 0 && goldPile.position[0] < gameMap->mapSize &&
            goldPile.position[1] >= 0 && goldPile.position[1] < gameMap->mapSize) {
            tempMap[goldPile.position[1]][goldPile.position[0]] = '*';
        }
    }

    // Serialize the temporary map into the buffer
    char* p = buffer;
    for (int i = 0; i < gameMap->mapSize; i++) {
        strncpy(p, tempMap[i], gameMap->mapSize + 1); // Copy each row including the newline
        p += gameMap->mapSize + 1; // Move the pointer by the size of the row plus newline
    }
    *p = '\0'; // Null-terminate the buffer

    // Free the temporary map
    for (int i = 0; i < gameMap->mapSize; i++) {
        free(tempMap[i]);
    }
    free(tempMap);

    strcat(playerInfo, buffer);

    free(buffer);

    return playerInfo; // Return the serialized buffer
}

