# CS50 Nuggets
## Design Spec
### Team Nuggets Extraordinaires: Jack, Sajjad, Emmanuel - Fall 2023

## Introduction
This document presents the design for the multiplayer exploration game "Nuggets". This game will allow multiple players to connect to a server, explore a map to collect gold nuggets, and compete to gather the most nuggets. Additionally, this game provides a spectator mode, allowing users to watch without direct interaction. Given the game's complexity and the interconnection of its various components, our design emphasizes breaking the system into manageable modules, structs, and features.

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.

## Team Structure
- **Jack (Developer 1)**: Focuses on the Server logic, game initialization, and map handling.
- **Sajjad (Developer 2)**: Manages the Client logic, display functions, and user interactions.
- **Emmanuel (Developer 3)**: Oversees network communication, protocol implementation, and error handling.

## Server
### User interface

The server's only interface with the user is on the command-line. It requires two arguments

* **map:** A text file holding the string representation of the game map
* **[seed]** (_optional_): A seed integer to use when randomly placing gold.

```bash=
$ ./server [map] [seed]
```

### Inputs and outputs

The server will receive inputs that will describe the state of the client
The *message* struct will be the key data structure for server receiving and sending messages:

##### Player Joined Message
When a player client starts, it shall send a message to the server:
```bash
PLAY [name]
```
##### Player Quit Message

```bash
QUIT [explanation]
```

##### OK
When a player has successfully been added to the game, the server will return an `OK` message back to that client
```bash
OK [A-Z]
```

##### Key Pressed Message
The client sends, at any time, the key pressed by the player
```bash
KEY [name] [key]
```

##### Spectator Joined Message
```bash
SPECTATE
```

##### Error Message
```bash
ERROR [explanation]
```

##### Grid
To initialize the client's window, the server sends a message containing the number of rows and columns in the map
```bash
GRID numRows numCols
```

##### Gold
To inform the player it has just collected n gold nuggets, its purse now has p gold nuggets, and there remain r nuggets left to be found.
```bash
GOLD n p r
```

##### Display
When the player's keystroke causes them to move to a new spot, the server shall inform all clients of a change in the game grid
```bash
DISPLAY\[map known to client]
```

### Functional decomposition into modules

* main: Parses arguments and begins run of the server with given parameters
* server_handle: Handles messages, sends messages, etc.
* initialize_game: Sets up data structures such as player struct, spectator struct, etc.
* parse_map: Reads the map file and sets up the data structures with it.
* game_over: Cleanup, free memory, and exit

### Pseudo code for logic/algorithmic flow

The server will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	call initializeGame() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
	call gameOver() to inform all clients the game has ended
	clean up


### Major data structures

### 1. Player
The `Player` struct represents each individual player in the game. It contains the following fields:
- **ID (Character)**: A unique identifier for each player. It can be used for referencing a specific player when sending messages or performing game operations.
- **Name (String, Max length 50)**: The name of the player. This could be a user-chosen nickname or a default name assigned by the game.
- **Position (X, Y)**: Coordinates representing the player's current location on the game map. It helps in determining player movements, interactions, and what part of the map is visible to them.
- **GoldCount (Integer)**: A counter that keeps track of how many gold nuggets a player has collected. It's crucial for determining the game's winner or leaderboard standings.
- **VisibleGrid (2D array)**: Represents the portion of the game map that the player can currently see.

### 2. GoldPile
The `GoldPile` struct represents collections of gold nuggets placed throughout the game map. It contains:
- **Position (X, Y)**: Coordinates showing where the gold pile is located on the game map. Players need to reach this position to collect the gold.
- **GoldCount (Integer)**: The number of gold nuggets in this pile. It will decrement as players collect gold from it.

### 3. GameMap
The `GameMap` struct provides a holistic view of the game's world and contains:
- **Grid (2D array)**: Represents the entire game map. Each cell in this array might contain information about whether it's a wall, a passage, a room, the presence of a player, gold, etc.
- **GoldPiles (List of GoldPile)**: A list containing all the gold piles in the game. This allows for easy tracking and updating of gold locations and quantities.
- **Players (set of Player)**: A set of all active players in the game. It helps in tracking player positions, scores, and interactions.

### 4. Spectator
- **ID (Character)**: “S” for Spectator.
- **Name (String, Max length 50)**: This will be “Spectator.”
- **Position (X, Y)**: Which will be set to Null.
- **GoldCount (Integer)**: This will also be Null.
- **VisibleGrid (2D array)**: This will be set to all coordinates.

### 5. Message
The `Message` struct is used for communication between the client and the server. It contains:
- **MessageType (Enum)**: Enumerated values like PLAY, SPECTATE, KEY, GRID, GOLD, DISPLAY, QUIT, ERROR that specify the kind of message or command being sent or received.
- **Content (String)**: The main content or payload of the message. Depending on the MessageType, it could contain player actions, game state updates, error messages, etc.