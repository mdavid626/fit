# ----------------------------------------------
# IZP - Projekt 2
# Encoding: UTF-8
# Soubor: Makefile
# Datum: 21.11.2010
# Autor: David Molnar
# Popis: Makefile pro sestaveni celeho projektu
# ----------------------------------------------

# make - sestaveni celeho projektu
# make clean - smazeni souburu *.o a spustitelni program
# make pack - komprimace celeho projektu do archivu login.zip

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -lm

EXEC = proj2

LOGIN = xmolna02

.PHONY: clean pack

all: $(EXEC)

proj2: proj2.o
	$(CC) $(CFLAGS) proj2.o -o proj2

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c proj2.c -o proj2.o

clean:
	rm -f *.o $(EXEC)

pack:
	zip $(LOGIN)-fit *.c *.txt Makefile
