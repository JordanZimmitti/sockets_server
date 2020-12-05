# Rule for building a .o file from a .cpp source file.
.SUFFIXES: .cpp
.cpp.o:
	$(CC) -c $(CXXFLAGS) $<

MAIN = ../server
CLIENT = client
CC = g++

# Compile with debug option and all warnings on.
CXXFLAGS = -g -std=c++11 -Wall -I../..

# Object modules comprising this application.
OBJ =  Mom.o Socket.o tools.o main.o

contain: $(OBJ) $(CLIENTOBJ)
	$(CC) -o $(MAIN) $(CXXFLAGS) $(OBJ)

clean:
	rm -f $(OBJ) $(MAIN)

tools.o: tools.cpp tools.hpp
Mom.o: Mom.cpp Mom.hpp Socket.hpp
Socket.o: Socket.hpp Socket.cpp shared.h tools.hpp Mom.hpp
main.o: main.cpp Mom.hpp Socket.hpp