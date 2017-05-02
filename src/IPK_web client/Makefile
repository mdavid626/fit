
# ----------------------------------------------
# IPK - Projekt 1
# Encoding: UTF-8
# Soubor: Makefile
# Datum: 25.03.2011
# Autor: David Molnar
# Popis: Makefile pro sestaveni celeho projektu
# ----------------------------------------------

# make - sestaveni celeho projektu
# make clean - smazeni souburu *.o a spustitelni program
# make pack - komprimace celeho projektu do archivu login.tar.gz

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

EXEC = webinfo

LOGIN = xmolna02

.PHONY: clean pack

all: $(EXEC)

proj4: webinfo.o
	$(CC) $(CFLAGS) webinfo.o -o webinfo

proj4.o: webinfo.c
	$(CC) $(CFLAGS) -c webinfo.c -o webinfo.o

clean:
	rm -f *.o $(EXEC)

pack:
	tar czvf $(LOGIN).tar.gz *.c Makefile
