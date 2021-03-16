# Makefile for sersniff
# Copyright 1999 Jonathan McDowell for Project Purple

CPPFLAGS += -g
#CFLAGS += -Wall -I/usr/local/include/ # uncomment to build locally
CFLAGS += -Wall
LINKFLAGS += -g -o
LDLIBS = -lzmq -lstdc++ -pthread
CXXFLAGS =

all : sersniff

sersniff: main.o sersniff.o tcp.o disp_basic.o
	$(CXX) $(CXXFLAGS) $(LINKFLAGS) main main.o sersniff.o tcp.o disp_basic.o $(LDLIBS) -o sersniff

clean :
	rm -f *~ core *.o a.out sersniff
