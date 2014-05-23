CC = gcc
CFLAGS =  -Wall -pedantic -std=c11 -g -D_GNU_SOURCE

AMStartup: AMStartup.o avatar.o
	@$(CC) $(CFLAGS) -o AMStartup src/AMStartup.o
	@echo "Building AMStartup"

avatar.o: src/avatar.c
	@$(CC) $(CFLAGS) -o avatar src/avatar.c `pkg-config --cflags --libs gtk+-2.0`
	@echo "Building avatar"

AMStartup.o: src/AMStartup.c src/amazing.h
	@$(CC) $(CFLAGS) -c src/AMStartup.c -o src/AMStartup.o
	@echo "Building src/AMStartup.o"

clean:
	@rm -f *~
	@rm -f *#
	@rm -f *.o
	@rm -f AMStartup
	@rm -f avatar
	@rm -f ./src/*~
	@rm -f ./src/*#
	@rm -f ./src/*.o
