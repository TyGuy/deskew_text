# This is a comment line
CC=g++
# CFLAGS will be the options passed to the compiler.
LDLIBS= -llept
CFLAGS= -c -Ileptonica
all: deskew
deskew: deskew.o
	$(CC) deskew.o -o deskew $(LDLIBS)
deskew.o: deskew.cpp
	$(CC) $(CFLAGS) deskew.cpp
clean:
	rm -rf *.o
