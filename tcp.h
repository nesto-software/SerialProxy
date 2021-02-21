/*
	tcp.h - Routines to give us handles to tcp ports.
	Copyright 1999 Project Purple. Written by Jonathan McDowell

	27/11/1999 - Started writing.
*/

#ifndef __TCP_H_
#define __TCP_H_

int opensock(char *host, int port);
int listensock(int port);

#endif /* __TCP_H_ */
