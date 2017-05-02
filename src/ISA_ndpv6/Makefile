# -----------------------------------------------------------------------------
# ISA Projekt
# Encoding: UTF-8 (without BOM)
# File: Makefile
# Date: 2011-11-01
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Description: Makefile to build the project
# -----------------------------------------------------------------------------

# make        - build the whole project
# make clean  - delete *.o and executable files
# make pack   - compress source files to xlogin00.tar

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -lpcap -lnet
LOGIN = xmolna02

.PHONY: clean pack

all: ndpv6

ndpv6: ndpv6.o
	$(CC) $(CFLAGS) ndpv6.o -o ndpv6

ndpv6.o: ndpv6.c
	$(CC) $(CFLAGS) -c ndpv6.c -o ndpv6.o

clean:
	rm -f *.o ndpv6

pack:
	tar cvf $(LOGIN).tar *.c Makefile Readme manual.pdf
