# ----------------------------------------------
# IZP - Projekt 4
# Encoding: UTF-8
# Soubor: Makefile
# Datum: 16.12.2010
# Autor: David Molnar
# Popis: Makefile pro sestaveni celeho projektu
# ----------------------------------------------

# make - sestaveni celeho projektu
# make clean - smazeni souburu *.o a spustitelni program
# make pack - komprimace celeho projektu do archivu login.zip

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

EXEC = proj4

LOGIN = xmolna02

.PHONY: clean pack

all: $(EXEC)

proj4: proj4.o
	$(CC) $(CFLAGS) proj4.o -o proj4

proj4.o: proj4.c
	$(CC) $(CFLAGS) -c proj4.c -o proj4.o

clean:
	rm -f *.o $(EXEC)

pack:
	zip $(LOGIN)-fit *.c *.txt Makefile
