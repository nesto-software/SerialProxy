# Makefile for sersniff
# Copyright 1999 Jonathan McDowell for Project Purple

CC = gcc
CPPFLAGS += -g
CFLAGS += -Wall -I/usr/local/include/
LINK = gcc
LINKFLAGS = -g -o
LDLIBS = -lzmq -lstdc++
CXXFLAGS =

all : sersniff

sersniff: sersniff.o tcp.o disp_basic.o
	$(LINK) $(CXXFLAGS) $(LINKFLAGS) sersniff sersniff.o tcp.o disp_basic.o $(LDLIBS)

clean :
	rm -f *~ core *.o a.out sersniff
