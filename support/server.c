#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

// Major Data Structures
typedef struct {
    char ID;
    char name[50];
    int position[2];
    int gold_count;
    char** visible_grid;
} Player;

typedef struct {
    int position[2];
    int gold_count;
} GoldPile;

typedef struct {
    char** grid;
    GoldPile* gold_piles;
    Player* players;
} GameMap;


void server_handle() {
    // TODO: Handle messages and game logic
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./server <map_filename>\n");
        return 1;
    }
    message_init(stderr);
    while (message_loop(NULL, 0, NULL, NULL, NULL)) {
        printf("test");
    }

    message_done();


    return 0;
}
