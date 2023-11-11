# CS50 Nuggets
## Implementation Spec
### Team 6, CS50, 2023

> This **template** includes some gray text meant to explain how to use the template; delete all of them in your document!

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes x, y, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor





### Data structures

The game implementation utilizes several key data structures to manage players, gold piles, empty spaces, and the overall game map.

### Player Structure

The `Player` structure represents an individual player in the game. Each player has a unique identifier (`ID`), a `name` (up to 50 characters), a current position on the game map (`position`), the total amount of gold collected (`gold_count`), gold picked up during the last move (`gold_picked`), a visible grid representing the portion of the map the player can see (`visible_grid`), and address information (`from`). Additionally, there's an array of characters (`characters[]`) to assign unique IDs to players.

```c
typedef struct {
    char ID;
    char name[50];
    int position[2];
    int gold_count;
    int gold_picked;
    char** visible_grid;
    addr_t from;
} Player;
```

### GoldPile Structure
The `GoldPile` structure represents a pile of gold on the game map. It includes the position of the gold pile (`position`) and the amount of gold in the pile (`gold_count`).

```c
typedef struct
{
    int position[2]; 
    int gold_count; 
} GoldPile;
```

### Empty structure
The `Empty` structure represents an empty space on the game map. It includes the `x` and `y coordinates` of the empty space.

```c
typedef struct
{
    int x; 
    int y; 
} Empty;
```

### GameMap structure
The `GameMap` structure encapsulates the entire state of the game. It includes the game grid (`grid`), an array of gold piles (`gold_piles`), the count of empty spaces (`emptySpaceCount`), an array of empty spaces (`emptySpaces`), an array of players (including up to 26 players and 1 spectator), the map size in rows (mapSizeR) and columns (`mapSizeC`), the number of gold piles (`numGoldPiles`), the total amount of gold remaining on the map (`goldLeft`), and port information (`port`).

```c
typedef struct
{
    char **grid;          
    GoldPile *gold_piles; 
    int emptySpaceCount;  
    Empty *emptySpaces;  
    Player *players[27]; 
    int mapSizeR;        
    int mapSizeC;      
    int numGoldPiles;    
    int goldLeft;        
    int port;          
} GameMap;
```


### Definition of function prototypes

## SERVER

We declared all the functions in declaration.h

```c
int main(int argc, char* argv[]);
bool handleMessage(void *arg, const addr_t from, const char *buf);
void game_over(GameMap *game_map);
void handle_player_join(GameMap *game_map, addr_t from, char *buf);
void collect_gold(Player* player, int newRow, int newCol, GameMap* game_map);
void handle_player_move(GameMap *game_map, addr_t from, char *moveDirectionStr);
void handle_quit(GameMap* game_map, addr_t from);
Empty* find_empty_spaces(char** grid, int row, int col, int* count);
void distribute_gold(GameMap *game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles);
GameMap *initialize_game(const char *map_filename, int seed);
char *serialize_map_with_players(GameMap *gameMap, addr_t from);
bool is_clear_path(GameMap* game_map, int start_x, int start_y, int end_x, int end_y);
bool line_of_sight(GameMap *game_map, int player_row, int player_col, int target_row, int target_col);
void calculate_visibility(GameMap* game_map, Player* player);
```

## CLIENT

```c
int main(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void init_ncurses(void);
static void end_ncurses(void);
```

### Detailed pseudo code

## SERVER

Our server-side code is distributed across several files; `server.c`, `player.c`, `gold.c`, `gamemap.c`, and `emptyspaces.c`.

#### server.c





#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player

---

## XYZ module

> For each module, repeat the same framework above.

### Data structures

### Definition of function prototypes

### Detailed pseudo code

---

## Testing plan

### unit testing

> How will you test each unit (module) before integrating them with a main program (client or server)?

### integration testing

> How will you test the complete main programs: the server, and for teams of 4, the client?

### system testing

> For teams of 4: How will you test your client and server together?

---

## Limitations

> Bulleted list of any limitations of your implementation.
> This section may not be relevant when you first write your Implementation Plan, but could be relevant after completing the implementation.
