# makefile
# Team 6
# Date: Nov 8, 2023
# Description: makefile for client and server

OBJS_server = server.o player.o
OBJS_client = client.o
LIBS = ./support/support.a
	
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I./support
CC = gcc
Make = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: client server
client:$(OBJS_client)
	$(MAKE) -C ./support
	$(CC) $(CFLAGS) $^ $(LIBS) -o client -lncurses

server:$(OBJS_server)
	$(MAKE) -C ./support
	$(CC) $(CFLAGS) $^ $(LIBS) -o server


client.o: client.c ./support/message.c ./support/log.c 
server.o: server.c ./support/message.h ./support/log.h 
player.o: player.h player.c struct.h ./support/message.h ./support/log.h

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f server
	rm -f client
	$(MAKE) -C ./support

