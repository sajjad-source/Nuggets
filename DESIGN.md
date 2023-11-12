# CS50 Nuggets
## Design Implementation Spec
### Team 6: Jack, Sajjad, Kasuti, CS50, Fall 2023

## Introduction
This document presents the design for the multiplayer exploration game "Nuggets". This game will allow multiple players to connect to a server, explore a map to collect gold nuggets, and compete to gather the most nuggets. Additionally, this game provides a spectator mode, allowing users to watch without direct interaction. Given the game's complexity and the interconnection of its various components, our design emphasizes breaking the system into manageable modules, structs, and features.

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.

## Team Structure
- **`Jack (Developer 1)`**: Focuses on the Server logic, game initialization, and map handling.
- **`Sajjad (Developer 2)`**: Manages the Client logic, display functions, and user interactions.
- **`Kasuti (Developer 3)`**: Oversees network communication, protocol implementation, and error handling.

## SERVER

## Program stucture
The program is structured into several functions and components, each responsible for specific tasks.

### Main Function
The `main` function serves as the entry point to server side. It does the following
1. Validates command-line inputs (map filename and optional seed).
2. Initializes the messaging system, handling errors.
3. Parses the seed, defaults to the process ID if not provided.
4. Initializes the game map with the given filename and seed.
5. Sets the port for the game map.
6. Enters a message loop, handling messages with the handleMessage function.

### Handle Messages
The `handleMessage` function manages incoming messages:
1. Extracts and processes commands like `PLAY` (for players), `SPECTATE` (for spectators), `KEY` (for player movement), and `QUIT` (for player quitting).
2. Invokes relevant functions to handle player joins, movements, and quits based on the received commands.
3. Sends serialized maps containing player positions to all connected players.
4. Checks for game completion by monitoring the remaining gold nuggets.


### Game over
The `game_over` function performs the following:
1. Constructs a game over message, including each player's ID, gold count, and name.
2. Sends the game over message to all connected players.


### Major data structures

The provided `struct.h` file defines key data structures used in the server-side implementation of the multiplayer game "Nuggets." Here's an overview of the structures:

1. **Player Structure (`Player`):**
   - `ID`: Unique identifier for the player.
   - `name`: Name of the player (up to 50 characters).
   - `position`: Current position of the player on the game map (x, y).
   - `gold_count`: Total gold count collected by the player.
   - `gold_picked`: Gold count picked up during the last move.
   - `visible_grid`: 2D array representing the visible portion of the game grid for the player.
   - `from`: Address information for the player.

2. **GoldPile Structure (`GoldPile`):**
   - `position`: Position of the gold pile on the game map (x, y).
   - `gold_count`: Gold count in the gold pile.

3. **Empty Structure (`Empty`):**
   - `x`: X-coordinate of an empty space.
   - `y`: Y-coordinate of an empty space.

4. **GameMap Structure (`GameMap`):**
   - `grid`: 2D array representing the game map.
   - `gold_piles`: Array of gold piles on the map.
   - `emptySpaceCount`: Number of empty spaces on the map.
   - `emptySpaces`: Array of empty spaces on the map.
   - `players`: Array to store player information (up to 26 players + 1 spectator).
   - `mapSizeR`: Number of rows in the game map.
   - `mapSizeC`: Number of columns in the game map.
   - `numGoldPiles`: Number of gold piles on the map.
   - `goldLeft`: Total amount of gold left on the map.
   - `port`: Port information for the game.

These structures provide a comprehensive representation of player data, gold piles, empty spaces, and the overall game map. 


## Implementation Strategy

**Initialization:**
- Jack is responsible for setting up the server, loading the map, and initializing the game state.
- Sajjad will handle client setup, manage communication with the server, and handle basic rendering.

**Core Gameplay Loop:**
- Jack will develop server-side logic for player movements, gold collection, and managing game end conditions.
- Sajjad will oversee the client-side game loop, capture user input, and handle display updates.

**Networking:**
- Kasuti will establish the UDP communication protocol, ensuring accurate message transmission between the server and clients.

**Error Handling and Edge Cases:**
- All developers will collaboratively address potential errors, covering issues such as malformed messages and unexpected user inputs.

**Testing and Refinement:**
- The team will conduct extensive testing, simulating various scenarios including different player counts, network conditions, and user inputs to ensure the game's robustness.

**Final Integration:**
- Developers will integrate all modules and features to ensure seamless communication and gameplay.

**Documentation:**
- Kasuti is in charge of documentation, ensuring comprehensive and detailed records for functions, structs, and modules, facilitating future reference and maintenance.

## CLIENT

## Program stucture
The program is structured into several functions and components, each responsible for specific tasks.

### Main Function
The `main` function serves as the entry point to server side. It does the following
1. Initialize the message module without logging.
2. Check and validate command-line arguments for hostname, port, and an optional player name.
3. Formulate the server address from the provided hostname and port.
4. Initialize the ncurses library for user interface.
5. Construct and send a "PLAY" or "SPECTATE" message to the server based on the command-line arguments.
6. Enter a loop, waiting for user input or incoming messages, using callback functions.
7. End the ncurses session and shut down the message module.

### Input Handling Function
1. Translate keypresses into server commands ("KEY [key]" for movement, "QUIT" for quitting).
2. Send the constructed command to the server.

### Message Handling Function
1. Clears the screen.
2. Prints the received message.
3. Refreshes the screen to display the new content.

### Ncurses Initialization 
1. Initializes the ncurses mode for a user-friendly interface.
2. Configures settings such as line buffering, keypad input, and color functionality.

### Ncurses Cleanup 
Ends the ncurses mode.

### More details
- The program uses the ncurses library for handling user input and creating a simple text-based UI.
- Messages received from the server are displayed on the client's screen.
- User input (keypresses) is translated into commands and sent to the server.
- The client can either play as a player with a specified name or spectate if no name is provided.

## Conclusion
This design outlines a structured approach to developing the "Nuggets" game. By distributing tasks among team members and segmenting the game into manageable modules, our team can efficiently develop, test, and refine the game. This approach guarantees an optimal user experience.