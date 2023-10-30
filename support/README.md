# support library

This library contains two modules useful in support of the CS50 final project.

## 'log' module

This module provides a simple way to log information to an output file.
See `log.h` for interface details, and `message.c` for some usage examples.
Each C file that includes `log.h` can call `message_init` with its own file descriptor; thus it is possible to output to different log files, or turn on/off logging independently.

## 'message' module

Provides a message-passing abstraction among Internet hosts.
See `message.h` for interface details, and the `UNIT_TEST` at the bottom of `message.c` for a simple usage example.

> **Note:** the unit test within `message.c` is not typical usage, because it supports a client and the server running the *same code*.
> More typically, the client and server programs will be separate programs, each with its own handlers.
> See the top of `message.h` for typical client and server structures.

Messages are sent via UDP and are thus limited to UDP packet size, may be lost, and may be reordered, but require no connection setup or teardown.
Within the Dartmouth campus network it is unlikely for messages to be lost or reordered; we will use this module as if neither will happen.

## compiling

To compile,

	make support.a

To clean,

	make clean

## using

In a typical use, assume this library is a subdirectory named `support`, within a directory where some main program is located.
The Makefile of that main directory might then include content like this:

```make
S = support
CFLAGS = ... -I$S
LLIBS = $S/support.a
LIBS =
...
program.o: ... $S/message.h $S/log.h
program: program.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
...
$S/support.a:
	make -C $S support.a

clean:
	make -C $S clean
	...
```

This approach allows the main program to be built (or cleaned) while automatically building (cleaning) the support library as needed.

## testing

The 'message' module has a built-in unit test, enabling it to be compiled stand-alone for testing.
See the `Makefile` for the compilation.

To compile,

	make messagetest

To run, you need two windows.
In the first window,

	./messagetest 2>first.log

In the second window,

	./messagetest 2>second.log localhost 12345

where `12345` is the port number printed by the first program.

Then you should be able to type a line in either window and, after pressing Return, see that message printed on the other.

The above example assumes both windows are on the same computer, which is known to itself as `localhost`.
Each window could be logged into a different computer, in which case the second above should provide the hostname or IP address of the first.
If the first is on the Thayer server known as "plank", the second would run

	./messagetest 2>second.log plank.thayer.dartmouth.edu 12345

On a private network, such as inside a home, you might only have an IP address of the first:

	./messagetest 2>second.log 10.0.1.13 12345

In all examples above notice we redirect the stderr (file number 2) to a log file, and we use different files for each instance... otherwise, if they are sharing a directory (as they would, on localhost), the log entries will overwrite each other.

## miniclient

The `miniclient` program is an example of the use of the message
module.  It acts as a client, and thus must be provided with the
address of a server.  Given the address of a server, this simple
client sends each line of stdin as a message to the server, and prints
to stdout every message received from the server; each printed message
is surrounded by 'quotes'.

## miniserver

The `miniserver` program is another example of the use of the message
module, albeit *not a good example*. Its purpose is to support testing
of clients.  It acts as a server, and prints the port number on which
it is waiting for messages.  It then receives and prints (to stdout) a
message and expects the user to enter one line of stdin, which it
sends back; each printed message is surrounded by 'quotes'.

This program is an atypical use of the message module because it
blocks on input from stdin during a call to handleMessage, rather than
using handleInput; this approach allows it to respond to each message
from each correspondent, one by one.
