#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "support/message.h"
#include <ncurses.h>

/**************** file-local functions ****************/
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void init_ncurses(void);
static void end_ncurses(void);

/***************** main *******************************/
int main(const int argc, char* argv[]) {
    // Initialize the message module (without logging)
    if (message_init(NULL) == 0) {
        return 2; // Failure to initialize message module
    }

    // Check arguments
    if (argc != 4 && argc != 3) { // Expecting one more argument for player's name
        fprintf(stderr, "usage: %s hostname port [playername]\n", argv[0]);
        return 3; // Bad command line
    }

    // Command line provides address for the server
    addr_t server; // Address of the server
    if (!message_setAddr(argv[1], argv[2], &server)) {
        fprintf(stderr, "Can't form address from %s %s\n", argv[1], argv[2]);
        return 4; // Bad hostname/port
    }

    // Initialize ncurses
    init_ncurses();

    // Construct the PLAY [name] message
    char playMsg[message_MaxBytes];

    if (argc == 3) {
        snprintf(playMsg, sizeof(playMsg), "PLAY %s", "Spectator");
    } else {
        snprintf(playMsg, sizeof(playMsg), "PLAY %s", argv[3]);
    }

    // Send the PLAY [name] message to the server
    message_send(server, playMsg);

    // Loop, waiting for input or for messages; provide callback functions.
    // We use the 'arg' parameter to carry a pointer to 'server'.
    bool ok = message_loop(&server, 0, NULL, handleInput, handleMessage);

    // End ncurses session
    end_ncurses();

    // Shut down the message module
    message_done();

    return ok ? 0 : 1; // Status code depends on result of message_loop
}

/**************** handleInput ****************/
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

/**************** handleMessage ****************/
static bool handleMessage(void* arg, const addr_t from, const char* message) {


    printf("TEST");
    // Clear the screen from the previous content
    clear();

    printw("%s", message);

    // Refresh the screen to show the new content
    refresh();

    return false;
}

/**************** init_ncurses ****************/
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