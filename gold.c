/** gold.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles gold pile
 */

// View declaration.h for more details

void distribute_gold(GameMap* game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles) {
    // Calculate map area
    int mapArea = game_map->mapSizeR * game_map->mapSizeC;

    // Define a maximum gold pile density (e.g., one pile per every 10 spaces)
    int maxDensity = 10;

    // Calculate the maximum number of gold piles based on map size and density
    int maxPilesBasedOnSize = mapArea / maxDensity;

    // Determine the number of gold piles (considering the map size)
    int numPiles = goldMinNumPiles + rand() % (goldMaxNumPiles - goldMinNumPiles + 1);
    numPiles = (numPiles < maxPilesBasedOnSize) ? numPiles : maxPilesBasedOnSize;
    numPiles = (numPiles < game_map->emptySpaceCount) ? numPiles : game_map->emptySpaceCount;

    game_map->gold_piles = malloc(numPiles * sizeof(GoldPile));
    if (game_map->gold_piles == NULL) {
        perror("Error allocating memory for gold piles");
        return;
    }

    // Distribute gold among the piles
    int remainingGold = goldTotal;
    for (int i = 0; i < numPiles; i++) {
        game_map->gold_piles[i].gold_count = 1;  // Assign at least one gold to each pile
        remainingGold--;
    }

    // Distribute the remaining gold
    for (int i = 0; i < remainingGold; i++) {
        int pileIndex = rand() % numPiles;
        game_map->gold_piles[pileIndex].gold_count++;
    }

    // Shuffle the empty spaces to randomize gold pile placement
    for (int i = 0; i < game_map->emptySpaceCount; i++) {
        int swapIndex = rand() % game_map->emptySpaceCount;
        Empty temp = game_map->emptySpaces[i];
        game_map->emptySpaces[i] = game_map->emptySpaces[swapIndex];
        game_map->emptySpaces[swapIndex] = temp;
    }

    // Place gold piles on the map
    for (int i = 0; i < numPiles; i++) {
        game_map->gold_piles[i].position[0] = game_map->emptySpaces[i].x;
        game_map->gold_piles[i].position[1] = game_map->emptySpaces[i].y;
    }

    game_map->numGoldPiles = numPiles;
    game_map->goldLeft = goldTotal;
}

