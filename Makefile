# Main makefile
CC = gcc
CFLAGS =  -Wall -pedantic -std=c11 -g -D_GNU_SOURCE
GTK = `pkg-config --cflags --libs gtk+-2.0`

# Make
MAKE = make --no-print-directory

# Library stuff
DIR = util/
UTILFLAG = -lamazingutil
UTILLIB = $(UTILDIR)libamazingutil.a

AMStartup: $(UTILLIB) avatar.o testing.o
	@$(CC) $(CFLAGS) -o AMStartup src/AMStartup.c -I $(DIR) -L $(DIR) $(UTILFLAG)
	@echo "Building AMStartup"

avatar.o: src/avatar.c
	@$(CC) $(CFLAGS) -o avatar src/avatar.c -I $(DIR) -L $(DIR) $(UTILFLAG) $(GTK)
	@echo "Building avatar"

testing.o: src/testing.c
	@$(CC) $(CFLAGS) -o testing src/testing.c -I $(DIR) -L $(DIR) $(UTILFLAG)
	@echo "Building testing"

$(UTILLIB):
	@$(MAKE) -C $(DIR)

clean:
	@rm -f *~
	@rm -f *#
	@rm -f *.o
	@rm -f AMStartup
	@rm -f avatar
	@rm -f testing
	@rm -f ./src/*~
	@rm -f ./src/*#
	@rm -f ./src/*.o
	@$(MAKE) -C $(DIR) clean
