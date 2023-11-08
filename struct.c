#include "support/message.h"

// Major Data Structures
typedef struct {
    char ID;
    char name[50];
    int position[2];
    int gold_count;
    char** visible_grid;
    addr_t from;
} Player;

typedef struct {
    int position[2];
    int gold_count;
} GoldPile;

typedef struct {
    int x;
    int y;
} Empty;

typedef struct {
    char** grid;
    GoldPile* gold_piles;
    int emptySpaceCount;
    Empty* emptySpaces;
    Player* players[27];
    int mapSize;
    int numGoldPiles;
    int goldLeft;
    int port;
} GameMap;


static int playerID = 0;
char characters[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};