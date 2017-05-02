# ------------------------------------------------------------------------------
# IPK - Project 3
# Encoding: UTF-8
# File: Makefile
# Date: 2011-04-29
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Description: Makefile to build the project
# ------------------------------------------------------------------------------

# make        - build the whole project
# make clean  - delete *.o and executable files
# make pack   - compress source files to xlogin00.tar.gz

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LOGIN = xmolna02

.PHONY: clean pack

all: server client

server: common.o rdtserver.o
	$(CC) $(CFLAGS) common.o rdtserver.o -o rdtserver

rdtserver.o: rdtserver.c
	$(CC) $(CFLAGS) -c rdtserver.c -o rdtserver.o

client: common.o rdtclient.o
	$(CC) $(CFLAGS) common.o rdtclient.o -o rdtclient

rdtclient.o: rdtclient.c
	$(CC) $(CFLAGS) -c rdtclient.c -o rdtclient.o

common.o: common.c
	$(CC) $(CFLAGS) -c common.c -o common.o

clean:
	rm -f *.o rdtserver rdtclient

pack:
	tar zcvf $(LOGIN).tar.gz *.c *.txt Makefile
