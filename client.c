/** 
 * client.c
 * Team 6
 * Date: Nov 8, 2023
 * Description: This file is responsible for implementing the client-side logic of the 'Nugget' multiplayer game. It handles the 
 * establishment of a connection to the game server, processes user inputs for game actions, and manages the display of the game state. 
 * This includes interpreting server messages, rendering the game map and player statuses, and handling player commands. The client 
 * utilizes the ncurses library for handling terminal-based input and output, providing a text-based interface for player interaction. 
 * This module is crucial for enabling players to interact with the game world, providing a seamless and intuitive user experience.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "support/message.h"
#include <ncurses.h>
#include "client.h"

int main(const int argc, char* argv[]) {
    // initialize the message module (without logging)
    if (message_init(NULL) == 0) {
        return 2; // failure to initialize message module
    }

    // check arguments
    if (argc != 4 && argc != 3) { // expecting one more argument for player's name
        fprintf(stderr, "usage: %s hostname port [playername]\n", argv[0]);
        return 3; // Bad command line
    }

    // command line provides address for the server
    addr_t server; // Address of the server
    if (!message_setAddr(argv[1], argv[2], &server)) {
        fprintf(stderr, "Can't form address from %s %s\n", argv[1], argv[2]);
        return 4; // Bad hostname/port
    }

    // initialize ncurses
    init_ncurses();

    // construct the PLAY [name] message
    char playMsg[message_MaxBytes];

    if (argc == 3) {
        snprintf(playMsg, sizeof(playMsg), "SPECTATE");
        
    } else {
        snprintf(playMsg, sizeof(playMsg), "PLAY %s", argv[3]);
    }

    // send the PLAY [name] message to the server
    message_send(server, playMsg);

    // loop, waiting for input or for messages; provide callback functions.
    // we use the 'arg' parameter to carry a pointer to 'server'.
    bool ok = message_loop(&server, 0, NULL, handleInput, handleMessage);

    // end ncurses session
    end_ncurses();

    // Shut down the message module
    message_done();

    return ok ? 0 : 1; // Status code depends on result of message_loop
}


/**************** handleInput() ****************/
/* see client.h for description */
static bool handleInput(void* arg) {
    // arg is a pointer to the server's address
    addr_t* server = (addr_t*)arg;

    int ch = getch();

    if (ch != ERR) { // ERR if no key is pressed

        // Translate the keypress to a server command
        // Construct the Move [key] message
        char command[message_MaxBytes];

        if (ch == 'h' || ch == 'l' || ch == 'j' || ch == 'k' || ch == 'u' || ch == 'y' || ch == 'b' || ch == 'n'
        || ch == 'H' || ch == 'L' || ch == 'J' || ch == 'K' || ch == 'U' || ch == 'Y' || ch == 'B' || ch == 'N') {
            snprintf(command, sizeof(command), "KEY %c", ch);
        } else if (ch == 'q') {
            snprintf(command, sizeof(command), "QUIT");
        } else {
            return false;
        }
        
        // Send the command to the server
        message_send(*server, command);
    }

    return false;
}

/**************** handleMessage() ****************/
/* see client.h for description */
static bool handleMessage(void* arg, const addr_t from, const char* message) {
    // Clear the screen from the previous content
    clear();

    printw("%s", message);

    // Refresh the screen to show the new content
    refresh();

    return false;
}

/**************** init_curses() ****************/
/* see client.h for description */
static void init_ncurses(void) {
    initscr();               // Start ncurses mode
    cbreak();                // Line buffering disabled
    keypad(stdscr, TRUE);    // Get F1, F2, etc...
    noecho();                // Don't echo while we do getch
    nodelay(stdscr, TRUE);   // Non-blocking getch
    curs_set(0);             // Don't display a cursor
    start_color();           // Start color functionality
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Initialize a color pair (pair number, foreground color, background color)
    attron(COLOR_PAIR(1));   // Apply color pair to window
}

/**************** end_ncurses ****************/
static void end_ncurses(void) {
    endwin(); // End ncurses mode
}