/** gold.c
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: C program that handles gold pile
 */

#include <stdlib.h>
#include "gold.h"
#include "gamemap.h"

// View gold.h for more details


// distributes gold piles on the game map
void distribute_gold(GameMap* game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles) {
    // calculate map area
    int mapArea = game_map->mapSizeR * game_map->mapSizeC;

    // define a scaling factor based on map size (e.g., mapArea / 100)
    // this factor will help determine the number of gold piles relative to map size
    float scalingFactor = (float)mapArea / 100;

    // calculate a proportional number of gold piles based on map size
    int proportionalPiles = (int)(scalingFactor * goldMaxNumPiles);
    proportionalPiles = (proportionalPiles < goldMinNumPiles) ? goldMinNumPiles : proportionalPiles;
    proportionalPiles = (proportionalPiles > goldMaxNumPiles) ? goldMaxNumPiles : proportionalPiles;

    // determine the final number of gold piles, ensuring it doesn't exceed empty spaces
    int numPiles = (proportionalPiles < game_map->emptySpaceCount) ? proportionalPiles : game_map->emptySpaceCount;

    // allocate memory for the gold piles
    game_map->gold_piles = malloc(numPiles * sizeof(GoldPile));
    if (game_map->gold_piles == NULL) {
        perror("Error allocating memory for gold piles");
        return;
    }

    // distribute gold among the piles
    int remainingGold = goldTotal;
    for (int i = 0; i < numPiles; i++) {
        game_map->gold_piles[i].gold_count = 1;  // Assign at least one gold to each pile
        remainingGold--;
    }

    // distribute the remaining gold
    for (int i = 0; i < remainingGold; i++) {
        int pileIndex = rand() % numPiles;
        game_map->gold_piles[pileIndex].gold_count++;
    }

    // shuffle the empty spaces to randomize gold pile placement
    for (int i = 0; i < game_map->emptySpaceCount; i++) {
        int swapIndex = rand() % game_map->emptySpaceCount;
        Empty temp = game_map->emptySpaces[i];
        game_map->emptySpaces[i] = game_map->emptySpaces[swapIndex];
        game_map->emptySpaces[swapIndex] = temp;
    }

    // place gold piles on the map
    for (int i = 0; i < numPiles; i++) {
        game_map->gold_piles[i].position[0] = game_map->emptySpaces[i].x;
        game_map->gold_piles[i].position[1] = game_map->emptySpaces[i].y;
    }

    // update the game map's gold-related attributes
    game_map->numGoldPiles = numPiles;
    game_map->goldLeft = goldTotal;
}

