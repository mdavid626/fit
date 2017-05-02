# ----------------------------------------------
# IZP - Projekt 1
# Encoding: UTF-8
# Soubor: Makefile
# Datum: 07.11.2010
# Autor: David Molnar
# Popis: Makefile pro sestaveni celeho projektu
# ----------------------------------------------

# make - sestaveni celeho projektu
# make clean - smazeni souburu *.o a spustitelni program
# make pack - komprimace celeho projektu do archivu login.zip

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

EXEC = proj1

LOGIN = xmolna02

.PHONY: clean pack

all: $(EXEC)

proj1: proj1.o
	$(CC) $(CFLAGS) proj1.o -o proj1

proj1.o: proj1.c
	$(CC) $(CFLAGS) -c proj1.c -o proj1.o

clean:
	rm -f *.o $(EXEC)

pack:
	zip $(LOGIN)-fit *.c *.txt Makefile
