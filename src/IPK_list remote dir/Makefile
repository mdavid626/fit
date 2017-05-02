# ------------------------------------------------------------------------------
# IPK - Projekt 2
# Encoding: UTF-8
# File: Makefile
# Date: 2011-04-15
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Description: Makefile to build the server and client
# ------------------------------------------------------------------------------

# make        - build the whole project: server and client
# make server - build only server
# make client - build only client
# make clean  - delete files *.o, server and client
# make pack   - compress source files and demo to xlogin00.tar.gz

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LOGIN = xmolna02

.PHONY: clean pack

all: server client

server: server.o
	$(CC) $(CFLAGS) server.o -o server

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -o server.o

client: client.o
	$(CC) $(CFLAGS) client.o -o client

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

clean:
	rm -f *.o server client

pack:
	tar czvf $(LOGIN).tar.gz *.c *.h *.pdf Makefile
