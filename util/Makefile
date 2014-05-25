# Util makefile
CC = gcc
CFLAGS = -Wall -pedantic -std=c11

# Make library
LIBNAME = libamazingutil.a
LIB = ar > /dev/null
LIBFLAGS = -cvq

make:
	@$(CC) $(CFLAGS) -c *.c 
	@$(LIB) $(LIBFLAGS) $(LIBNAME) *.o
	@echo "Building library"

clean:
	@rm -f $(LIBNAME)
	@rm -f *.o
