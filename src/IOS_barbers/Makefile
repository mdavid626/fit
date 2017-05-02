# ------------------------------------------------------------------------------
# IOS - Projekt 2
# Encoding: UTF-8
# File: Makefile
# Date: 2011-05-01
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Description: Makefile to build the project
# ------------------------------------------------------------------------------

# make        - build the whole project
# make clean  - delete *.o and executable files
# make pack   - compress source files to xlogin00.zip

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LOGIN = xmolna02

.PHONY: clean pack

all: barbers

barbers: barbers.o
	$(CC) -lrt $(CFLAGS) barbers.o -o barbers

barbers.o: barbers.c
	$(CC) -lrt $(CFLAGS) -c barbers.c -o barbers.o

clean:
	rm -f *.o barbers

pack:
	zip $(LOGIN).zip *.c Makefile
