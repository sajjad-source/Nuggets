/** gold.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles gold pile
 */

// View declaration.h for more details

void distribute_gold(GameMap* game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles) {

    // Determine the number of gold piles
    int numPiles = goldMinNumPiles + rand() % (goldMaxNumPiles - goldMinNumPiles + 1);

    // Allocate memory for the gold piles
    game_map->gold_piles = malloc(numPiles * sizeof(GoldPile));
    if (game_map->gold_piles == NULL) {
        perror("Error allocating memory for gold piles");
        return;
    }

    // Distribute gold among the piles
    int remainingGold = goldTotal;
    for (int i = 0; i < numPiles; i++) {
        // Ensure each pile gets at least 1 gold piece
        int goldInPile = (remainingGold > (numPiles - i)) ? (1 + rand() % (remainingGold - (numPiles - i) + 1)) : 1;
        game_map->gold_piles[i].gold_count = goldInPile;
        remainingGold -= goldInPile;
    }

    printf("This is gold remainging: %d\n", remainingGold);

    // Place gold piles on the map
    for (int i = 0; i < numPiles; i++) {
        if (i < game_map->emptySpaceCount) {

            // Select a random empty space index
            int randomIndex = rand() % game_map->emptySpaceCount;

            // Assign the position of the gold pile to the randomly selected empty space
            game_map->gold_piles[i].position[0] = game_map->emptySpaces[randomIndex].x;
            game_map->gold_piles[i].position[1] = game_map->emptySpaces[randomIndex].y;

            // Swap the selected empty space with the last one in the array and decrease the count
            game_map->emptySpaces[randomIndex] = game_map->emptySpaces[game_map->emptySpaceCount - 1];
            game_map->emptySpaceCount--;
        }
    }

    // Update the game map with the number of gold piles and the total gold remaining
    game_map->numGoldPiles = numPiles;
    game_map->goldLeft = goldTotal;
}
