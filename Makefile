# Makefile for sersniff
# Copyright 1999 Jonathan McDowell for Project Purple

CPPFLAGS += -g
#CFLAGS += -Wall -I/usr/local/include/ # uncomment to build locally
CFLAGS += -Wall
LINK = gcc
LINKFLAGS = -g -o
LDLIBS = -lzmq -lstdc++
CXXFLAGS =

all : sersniff

sersniff: sersniff.o tcp.o disp_basic.o
	$(LINK) $(CXXFLAGS) $(LINKFLAGS) sersniff sersniff.o tcp.o disp_basic.o $(LDLIBS)

clean :
	rm -f *~ core *.o a.out sersniff
