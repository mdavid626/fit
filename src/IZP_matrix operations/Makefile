# ----------------------------------------------
# IZP - Projekt 3
# Encoding: UTF-8
# Soubor: Makefile
# Datum: 02.12.2010
# Autor: David Molnar
# Popis: Makefile pro sestaveni celeho projektu
# ----------------------------------------------

# make - sestaveni celeho projektu
# make clean - smazeni souburu *.o a spustitelni program
# make pack - komprimace celeho projektu do archivu login.zip

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

EXEC = proj3

LOGIN = xmolna02

.PHONY: clean pack

all: $(EXEC)

proj3: proj3.o
	$(CC) $(CFLAGS) proj3.o -o proj3

proj3.o: proj3.c
	$(CC) $(CFLAGS) -c proj3.c -o proj3.o

clean:
	rm -f *.o $(EXEC)

pack:
	zip $(LOGIN)-fit *.c *.txt Makefile
