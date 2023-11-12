/** client.h
 *  Team 6
 *  Date: Nov 8, 2023
 *  Description: h file for client.c
*/

#ifndef CLIENT_H
#define CLIENT_H
/**************** handleInput ****************/
/**
 * Caller provides:
 * 	valid arg
 *
 * TODO:
 *  arg is a pointer to the server's address
 *  Translate the keypress to a server commanD
 *  Construct the Move [key] message
 *  Send the command to the server

 * RETURN:
 *  false
*/
static bool handleInput(void* arg);


/**************** handleMessage ****************/
/**
 * Caller provides:
 * 	valid arg, valid addr_t pointer and valid message
 *
 * TODO:
 *  Clear the screen from the previous content
 *  Print message to client
 *  Refresh the screen to show the new content

 * RETURN:
 *  false
*/
static bool handleMessage(void* arg, const addr_t from, const char* message);


/**************** init_ncurses ****************/
/**
 * Caller provides:
 * 	is void
 *
 * TODO:
 *  Start ncurses mode
 *  Disable line buffering
 *  Get F1, F2, etc...
 *  Don't echo while we do getch
 *  Non-blocking getch
 *  Don't display a cursor
 *  Start color functionality
 *  Initialize a color pair
 *  Apply color pair to window
*/
static void init_ncurses(void);


/**************** end_ncurses ****************/
/**
 * Caller provides:
 * 	is void
 *
 * TODO:
 *  use endwin() to end ncurses
*/
static void end_ncurses(void);

#endif //CLIENT_H