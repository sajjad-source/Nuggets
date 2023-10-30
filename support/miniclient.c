/* 
 * miniclient - a simple client using the messaging module
 *
 * Given the address of a server, this simple client sends each line of stdin
 * as a message to the server, and prints to stdout every message received
 * from the server; each printed message is surrounded by 'quotes'.
 * 
 * David Kotz - May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include "message.h"

/**************** file-local functions ****************/

static bool handleInput  (void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);

/***************** main *******************************/
int
main(const int argc, char* argv[])
{
  // initialize the message module (without logging)
  if (message_init(NULL) == 0) {
    return 2; // failure to initialize message module
  }

  // check arguments
  const char* program = argv[0];
  if (argc != 3) {
    fprintf(stderr, "usage: %s hostname port\n", program);
    return 3; // bad commandline
  }
  
  // commandline provides address for server
  const char* serverHost = argv[1];
  const char* serverPort = argv[2];
  addr_t server; // address of the server
  if (!message_setAddr(serverHost, serverPort, &server)) {
    fprintf(stderr, "can't form address from %s %s\n", serverHost, serverPort);
    return 4; // bad hostname/port
  }

  // Loop, waiting for input or for messages; provide callback functions.
  // We use the 'arg' parameter to carry a pointer to 'server'.
  bool ok = message_loop(&server, 0, NULL, handleInput, handleMessage);

  // shut down the message module
  message_done();
  
  return ok? 0 : 1; // status code depends on result of message_loop
}

/**************** handleInput ****************/
/* stdin has input ready; read a line and send it to the client.
 * Return true if the message loop should exit, otherwise false.
 * i.e., return true if EOF was encountered on stdin, or fatal error.
 */
static bool
handleInput(void* arg)
{
  // We use 'arg' to receive an addr_t referring to the 'server' correspondent.
  // Defensive checks ensure it is not NULL pointer or non-address value.
  addr_t* serverp = arg;
  if (serverp == NULL) {
    fprintf(stderr, "handleInput called with arg=NULL");
    return true;
  }
  if (!message_isAddr(*serverp)) {
    fprintf(stderr, "handleInput called without a correspondent.");
    return true;
  }
  
  // allocate a buffer into which we can read a line of input
  // (it can't be any longer than a message)!
  char line[message_MaxBytes];

  // read a line from stdin
  if (fgets(line, message_MaxBytes, stdin) == NULL) {
    // EOF case: stop looping
    return true;
  } else {
    // strip trailing newline
    const int len = strlen(line);
    if (len > 0 && line[len-1] == '\n') {
      line[len-1] = '\0';
    }

    // send as message to server
    message_send(*serverp, line);

    // normal case: keep looping
    return false;
  }
}

/**************** handleMessage ****************/
/* Datagram received; print it.
 * We ignore 'arg' here.
 * Return true if any fatal error.
 */
static bool
handleMessage(void* arg, const addr_t from, const char* message)
{
  printf("'%s'\n", message);
  fflush(stdout);
  return false;
}
