# Makefile for sersniff
# Copyright 1999 Jonathan McDowell for Project Purple

CC = gcc
CFLAGS += -g -Wall
LINK = gcc
LINKFLAGS = -g -o

all : sersniff

sersniff: sersniff.o tcp.o disp_basic.o
	$(LINK) $(LINKFLAGS) sersniff sersniff.o tcp.o disp_basic.o

clean :
	rm -f *~ core *.o a.out sersniff
