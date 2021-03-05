/*
	tcp.c - Routines to give us handles to tcp ports.
	Copyright 1999 Project Purple. Written by Jonathan McDowell

	27/11/1999 - Started writing.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

int opensock(char *host, int port)
{
	int sock;
	struct sockaddr_in saddr;
	struct hostent *hoste;

	if ((hoste=gethostbyname(host))==NULL) {
		perror("gethostbyname()");
		return -1;
	}

	if ((sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		perror("socket()");
		return -1;
	}

	saddr.sin_addr = *(struct in_addr *) hoste->h_addr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	if ((connect(sock, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)))==-1) {
		perror("connect()");
		return -1;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK)==-1) {
		perror("fcntl()");
		return -1;
	}

	return sock;
}

int listensock(int port)
{
	int sock, newsock;
	struct sockaddr_in saddr;
	socklen_t saddrlen;
	int on;

	if ((sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		perror("socket()");
		return -1;
	}

	on=1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("setsockopt(SO_REUSEADDR)");
		return -1;
	}

	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddrlen = sizeof(saddr);

	if (bind(sock, (struct sockaddr *) &saddr, saddrlen)==-1) {
		perror("bind()");
		return -1;
	}

	if (listen(sock, 0)==-1) {
		perror("listen()");
		return -1;
	}

	if ((newsock=accept(sock, (struct sockaddr *) &saddr, &saddrlen))==-1) {
		perror("accept()");
		return -1;
	}

	if (fcntl(newsock, F_SETFL, O_NONBLOCK)==-1) {
		perror("fcntl()");
		return -1;
	}

	close(sock);

	return newsock;
}
