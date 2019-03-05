CC = gcc
DEBUGFLAGS = -O2 -std=c99 -g -pg -Wall -pedantic 
FLAGS = -O2 -std=c99 -Wall -pedantic 
POST = -lm

clean:
	rm erodr

debug:
	$(CC) $(DEBUGFLAGS) src/main.c -o erodr $(POST)

build:
	$(CC) $(FLAGS) src/main.c -o erodr $(POST)
