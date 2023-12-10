# Nuggets
## Design README.md
### Team 6: Jack, Sajjad, Kasuti

## Overview
This repository hosts the code for the "Nuggets" game, where players explore a set of rooms and passageways in search of gold nuggets. The game's map is defined and loaded by the server at the start. Gold nuggets are distributed randomly in piles within the rooms. Up to 26 players, along with one spectator, can participate in a game. Each player is dropped randomly into a room upon joining. Players move around, collecting nuggets when they move onto a pile. The game concludes when all gold nuggets are collected, and a summary is printed.

## Team Structure
- **`Jack (Developer 1)`**: Focuses on the Server logic, game initialization, and map handling.
- **`Sajjad (Developer 2)`**: Manages the Client logic, display functions, and user interactions.
- **`Kasuti (Developer 3)`**: Oversees network communication, protocol implementation, and error handling.

## Usage

#### Server
The server program accepts two or three command-line arguments:

```bash
./server map_filename [seed]
```

- `map_filename`: Specify the filename of the map to be used for the game.
- `seed`: If no seed is provided, it defaults to a predefined value.

#### Client
The client program requires three or four command-line arguments:

```bash
./client hostname port [playername]
```

- `hostname`: Specify the server's hostname.
- `port`: Specify the port number for communication.
- `playername`: Optionally, provide a player name when joining as a player. If not provided, the client joins as a spectator.


## Builing and running
To build the server and client, use the provided Makefile:

```bash
$ make 
```

To clean run:

```bash
$ make clean
```