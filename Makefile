CC = gcc
FLAGS = -pedantic -Wall

all: reserver reclient

reserver: reserver.c
	$(CC) $(FLAGS) reserver.c -o reserver

reclient: reclient.c
	$(CC) $(FLAGS) reclient.c -o reclient

.PHONY: clean

clean:
	rm -f reserver reclient *.o
