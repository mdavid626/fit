# -----------------------------------------------------------------------------
# ISA Projekt
# Encoding: UTF-8
# File: Makefile
# Date: 2015-11-17
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Description: Makefile to build the project
# -----------------------------------------------------------------------------

# make        - build the whole project
# make clean  - delete *.o and executable files
# make pack   - compress source files to xmolna02.tar

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra  -pedantic 
#-Werror
LOGIN = xmolna02

.PHONY: clean pack

all: myripsniffer myripresponse

myripsniffer: myripsniffer.o
	$(CC) $(CFLAGS) myripsniffer.o -o myripsniffer -lpcap

myripsniffer.o: myripsniffer.c
	$(CC) $(CFLAGS) -c myripsniffer.c -o myripsniffer.o
    
myripresponse: myripresponse.o
	$(CC) $(CFLAGS) myripresponse.o -o myripresponse -lpcap

myripresponse.o: myripresponse.c
	$(CC) $(CFLAGS) -c myripresponse.c -o myripresponse.o

clean:
	rm -f *.o myripsniffer myripresponse

pack:
	tar cvf $(LOGIN).tar *.c *.h Makefile README manual.pdf
