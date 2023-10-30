/* 
 * message - a UDP-based messaging module
 *
 * Provides a message-passing abstraction among Internet hosts.  Messages
 * are sent via UDP and are thus limited to UDP packet size, may be lost,
 * and may be reordered, but require no connection setup or teardown.
 * 
 * Typical server sequence looks like this:
 *   message_init(stderr);
 *   message_loop(arg, timeout, handleTimeout, handleStdin, handleMessage);
 *   message_done();
 * Typical client sequence looks like this:
 *   message_init(stderr);
 *   message_setAddr(serverHost, serverPort, &serverAddress);
 *   message_send(serverAddress, message); // client speaks first
 *   message_loop(arg, timeout, handleTimeout, handleStdin, handleMessage);
 *   message_done();
 * Note:
 *  handleTimeout may be NULL (and timeout==0) if no timers needed.
 *  handleInput may be NULL if no input expected.
 *  arg may be NULL if not needed by handlers.
 *
 * David Kotz - May 2019
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>  // These two includes are not needed for this file, 
#include <sys/select.h> // but is needed for users of this file.

/****************** types *********************/
/* A type representing an Internet address, suitable for use in message_send().
 * Although not technically opaque (because struct sockaddr_in is well
 * documented and not made opaque by the include file <arpa/inet.h> that
 * defines it), users of this module should treat addr_t as an opaque type.
 * Module users can declare variables of type addr_t, and initialize them
 * to the value returned by message_noAddr, or initialize them in a call to
 * message_setAddr, or receive them as a parameter in one of the handler
 * functions.  Addresses can be passed by value to and from functions, but they
 * cannot be compared directly for equality; to compare two addresses,
 * use message_eqAddr.
 */
typedef struct sockaddr_in addr_t;

/****************** constants *********************/
// Maximum payload size for UDP messages, according to
// https://en.wikipedia.org/wiki/User_Datagram_Protocol
static const int message_MaxBytes = 65507;

/****************** global functions *********************/

/******************************************/
/* message_init: initialize the module.
 * Caller provides:
 *   file pointer(fp), passed through to log_init().  May be NULL.
 * Function returns:
 *   port number where messages can be sent; zero on error.
 * Caller expectations:
 *   call message_done() later when all messaging operations complete.
 * Logs: information about errors; the port number.
 */
int message_init(FILE* logFP);

/******************************************/
/* message_noAddr: return an addr_t representing "no address".
 * Logs: nothing.
 */
addr_t message_noAddr(void);

/******************************************/
/* message_isAddr: is the given address a valid address?
 * Caller provides: an address.
 * Function returns: true iff the address appears to be valid.
 * Notes:
 *   This function does not verify the IP address or port number.
 *   Its main purpose is to verify the address is not 'no address'; thus,
 *    message_isAddr(message_noAddr()) == false;
 * Logs: nothing.
 */
bool message_isAddr(const addr_t addr);

/******************************************/
/* message_eqAddr: are two addresses equal?
 * Caller provides: two addresses
 * Function returns: true iff the addresses are identical.
 * Logs: nothing.
 */
bool message_eqAddr(const addr_t a, const addr_t b);

/******************************************/
/* message_setAddr: initialize an address to a given hostname and port.
 * Caller provides: 
 *   a string representing the hostname, or numeric IP address.
 *   a string representing the port number.
 *   a pointer to an address, which will be initialized.
 * Function returns: 
 *   true if successful in initalizing the address;
 *   false if an error, which may indicate a bad hostname or port number.
 * Logs:
 *   information about errors in parameters,
 *   information about errors in the hostname or port number.
 */
bool message_setAddr(const char* hostname, const char* portStr, addr_t* addr);

/******************************************/
/* message_stringAddr:
 * Produce a string representation of the address.
 * This function is primarily useful for debugging, e.g.,
 *    printf("The network address is %s\n", message_stringAddr(address));
 * Caller provides:
 *   an address.
 * Returns:
 *   a string representation of the address,
 *   which is a pointer to static storage that cannot be retained!
 * Logs:
 *   nothing.
 */
const char* message_stringAddr(const addr_t addr);

/******************************************/
/* message_send: send a message.
 * Caller provides:
 *   a valid address to which to send the message,
 *   a string containing the message.
 * Function returns: none
 * Assumptions: message_init() has already been called.
 * Logs:
 *   errors in arguments,
 *   errors in sending the message.
 */
void message_send(const addr_t to, const char* message);

/******************************************/
/* message_loop: loop, handling input and incoming messages.
 * Caller provides:
 *   a pointer for an arg (may be NULL), passed to the handler functions,
 *   a time duration (in seconds) after which to call "timeout" (ignore if 0),
 *   a function for handling a timeout (may be NULL),
 *   a function for handling input from stdin (may be NULL),
 *   a function for handling an inbound message (may be NULL).
 * Function returns:
 *   true, in the normal case when the loop ends due to handler return true;
 *   false, when fatal errors indicate we cannot keep looping.
 * Handlers:
 *   handleTimeout: called when time passes without input or message.
 *   handleInput: should read once from stdin and process it.
 *   handleMessage: provided the address from which the message arrived,
 *     and a string containing the contents of the message. The handler should
 *     realize the string's memory will be reused upon return from the handler.
 *   All are provided 'arg', passed-through untouched.
 *   Handlers should return true to terminate looping, false to keep looping.
 * Notes:
 *   The timeout feature is optional; use timeout=0 and handleTimeout=NULL.
 * Logs:
 *   errors in arguments,
 *   errors in monitoring stdin and/or network,
 *   sender's address and content of every message received.
 */
bool message_loop(void* arg, const float timeout,
                  bool (*handleTimeout)(void* arg),
                  bool (*handleInput)  (void* arg),
                  bool (*handleMessage)(void* arg,
                                        const addr_t from, 
                                        const char* message));

/******************************************/
/* message_done: shut down the module.
 * Caller provides: nothing.
 * Function returns: nothing.
 * Assumptions: 
 *   message_init() had been called earlier.
 *   no message() functions will be called later.
 * Logs: a note indicating close down of message module.
 */
void message_done(void);


#endif // _MESSAGE_H_
