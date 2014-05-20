CC = gcc
CFLAGS =  -Wall -pedantic -std=c11 -g -D_GNU_SOURCE

AMStartup: AMStartup.o
	@$(CC) $(CFLAGS) -o AMStartup AMStartup.o
	@echo "Building AMStartup"

AMStartup.o: src/AMStartup.c src/amazing.h
	@$(CC) $(CFLAGS) -c src/AMStartup.c
	@echo "Building /src/AMStartup.o"

clean:
	@rm -f *.o
	@rm -f AMStartup
