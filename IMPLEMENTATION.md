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

### Detailed pseudo code

Our server-side code is distributed across several files; `server.c`, `player.c`, `gold.c`, `gamemap.c`, and `emptyspaces.c`.

#### server.c

### handleMessage
    Initialize game
        Duplicate the message buffer to use with strtok
        Check if the duplication was successful
            Extract the command from the message
            Handle the "Join" command
            Handle the "Move" command
        Update and send the serialized map to all players
            Serialize the map with players and send it to each player
        Check if all gold is collected, and end the game if true
    return false to continue the message loop

### game_over
    Create a buffer to store the game over message
        Initialize the buffer
        Concatenate the initial game over message
        Iterate through players and append their information to the game over message
        Send the game over message to all players

#### player.c

### handle_player_join
    Check if the player already exists by matching the 'from' address
        If the player already exists, do nothing 
        Case when a spectator joins
            Allocate memory for a new spectator player
            Initialize the spectator's attributes
            If there was an existing spectator, inform them about replacement
            Add the spectator player to the last slot in the players array
        Case when a player joins
            Check if there is space for a new player
            Allocate memory for a new player
            Initialize the new player's attributes
            Place the player on a random empty space on the map
            Update the game map with the new player
            Allocate memory for the player's visible grid
            Check if player spawns on a gold pile, if so, collect it and spawn the player
            Increment the playerID for the next player
        else: Inform the player that the game is full

### collect_gold
    If the new position is a gold pile, update the gold count and remove the pile
        Check if the new position matches the position of a gold pile
            Increase player's gold count and the amount of gold picked up
            Decrease the overall count of gold left in the game
            Remove the gold pile by swapping it with the last one in the array (if not already the last)
        Decrease the count of gold piles on the map

### handle_player_move
    Extract move direction from the buffer
        Find the player with the given ID
        If the player is not found, print an error and return
        Calculate new position based on move direction
        Determine whether the movement should be continuous or not
        Update player's position while collecting gold
            h - left, l - right, j - down, k - up
            y - diagonally up-left u - diagonally up-right, 
            b - diagonally down-left n - diagonally down-right
        Check if the new position is within the bounds of the map and not occupied
            Update player's position
            Collect gold at the new position
        else: Invalid move

### handle_quit
    find the player with the given ID
        if the player is found, update their position to indicate they have quit
        send the quit message to player / spectator


### emptyspaces.c

### find_empty_spaces
    Allocate memory for an array to store empty space coordinates
        Initialize the count of empty spaces to zero
        Iterate through each cell in the grid to find empty spaces
            Check if the current cell is an empty space ('.')
                Store the coordinates of the empty space in the array
                Increment the count of empty spaces
            Shrink the array to the number of empty spaces found
    return an array of empty space coordinates

### gold.c

### distribute_gold
    Calculate map area
        Define a scaling factor based on map size (e.g., mapArea / 100)
        This factor will help determine the number of gold piles relative to map size
        Calculate a proportional number of gold piles based on map size
        Determine the final number of gold piles, ensuring it doesn't exceed empty spaces
        Allocate memory for the gold piles
        Distribute gold among the piles: Ensure each pile gets at least 1 gold piece
        Distribute the remaining gold
        Shuffle the empty spaces to randomize gold pile placement
        Place gold piles on the map
        Update the game map's gold-related attributes

### gamemap.c

### initialize_game
    Seed the random number generator
        Open the map file
        Count the number of characters until a newline
        Rewind to the start of the file to read the grid
        Allocate memory for the GameMap structure
        Set the size of the map
        Allocate memory for the grid
        Allocate memory for each row in the grid
        Read the map from the file
        Find empty spaces after loading the map
        Check if there are no empty spaces on the map
        distribute the gold
    return gamemap

### serialize_map_with_players
    Calculate buffer size: one char for each cell plus one for each newline, 
        plus one for the null terminator
        Create a copy of the map
            Handle allocation failure
        Overlay players' positions on the temporary map
        Overlay gold piles positions on the temporary map
        Serialize the temporary map into the buffer
        Free the temporary map
    return serialized buffer [playerInfo]


### is_clear_path
    Check if we've reached the target point
        Check if the current position is a wall
            If the wall is visible return true
            else break (reached the target)
        Check if the current position is a clear path
            return false (not clear)
    return true if path is clear


### line_of_sight
    If directly adjacent spots are visible, return true
    Check if there is a clear path to the target spot
    return true if adjacent spots are visible


### calculate_visibility
    Create a visible grid for the player with the same size as the game_map
        Check visibility for every cell in the grid
            For walls, check adjacent tiles as well as direct line of sight
            Ensure that we don't check out of bounds or through corners
                Diagonal checks are through corners, so we skip them
                Break out of the inner loop on first visible
            Break out of the outer loop if visible
        For non-wall tiles, use direct line of sight check

## CLIENT

### Definition of function prototypes

```c
int main(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void init_ncurses(void);
static void end_ncurses(void);
```

### Detailed pseudo code

### main
    Initialize the message module (without logging)
    Check arguments
    Command line provides address for the server
    Initialize ncurses
    Construct the PLAY [name] message
    Send the PLAY [name] message to the server
    Loop, waiting for input or for messages; provide callback functions
    We use the 'arg' parameter to carry a pointer to 'server'
    End ncurses session
    Shut down the message module
    Status code depends on result of message_loop

### handleInput
    arg is a pointer to the server's address
    Translate the keypress to a server commanD
    Construct the Move [key] message
    Send the command to the server

### handleMessage
    Clear the screen from the previous content
    Print message to client
    Refresh the screen to show the new content

### init_ncurses
    Start ncurses mode
    Disable line buffering
    Get F1, F2, etc...
    Don't echo while we do getch
    Non-blocking getch
    Don't display a cursor
    Start color functionality
    Initialize a color pair
    Apply color pair to window

### end_ncurses
    use endwin() to end ncurses


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
