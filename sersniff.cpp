/*
	sersniff.c - A program to tunnel between 2 serial ports and
	sniff the data that passes between them. Designed to aid
	working out the protocol between a Nokia 9000i and NServer.
	Written by Jonathan McDowell for Project Purple, 1999
	Extra stuff by Cornelius Cook (cook@cpoint.net), 1999
	OSX support and EOF support by Pete Baker (peteb4ker@gmail.com), 2011

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software Foundation
	Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
	http://www.gnu.org/copyleft/gpl.html

	07/09/1999 - Started writing.
	21Nov1999  - Cook: added command line support and extra error checking
	27Nov1999  - Cook: added select, timer & changed output look
	24Jun2011 - Baker: Added OSX support. Added EOF character support
*/

#define VERSION "0.0.5"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <zmq.hpp>

#include "disp_basic.h"
#include "tcp.h"

zmq::context_t *ctx = new zmq::context_t();
zmq::socket_t sock (*ctx, zmq::socket_type::pub);

char * speed_str[] = { "300", "1200", "2400", "4800", "9600", "19200", "38400",
		"57600", "115200", "230400", NULL };
speed_t speed_num[] = { B300, B1200, B2400, B4800, B9600, B19200, B38400,
		B57600, B115200, B230400, B0 };

void send_via_zmq(char payload) {

	std::string str(1, payload);
	sock.send(zmq::message_t(str), zmq::send_flags::dontwait); 
}

int openport(const char *device, speed_t baud, int setup)
{
	int filedes;
	struct termios serparams;

	if ((filedes=open(device, O_RDWR | O_NONBLOCK))==-1) {
		/* Can't open device */
		fprintf(stderr,"%s: ",device);
		perror(device);
		exit(1);
	}

	if (setup) {
		bzero(&serparams, sizeof(serparams));

		cfsetspeed(&serparams, baud);
		serparams.c_cflag |= CLOCAL | CS8 | CREAD;

		if (tcflush(filedes, TCIFLUSH)) {
			fprintf(stderr,"%s: ",device);
			perror("tcflush");
			exit(1);
		}
		if (tcsetattr(filedes, TCSANOW, &serparams)) {
			fprintf(stderr,"%s: ",device);
			perror("tcsetattr");
			exit(1);
		}
	}

	return filedes;
}

int closeport(int filedes)
{
	/* Should remove any locks at this point */
	return close(filedes);
}

/*
this returns the string for the character passed to it
It could be expanded in the future to maybe to string recognitions?
*/
char *chardecide(unsigned char c, int alpha, char *format) {
	static char result[256];

	/* everyone should take up 5 characters */
	if (alpha) {
	if ((c < 32) | (c > 126)) {
		switch (c) {
			case 10:
				sprintf(result,"<LF>");
				break;
			case 13:
				sprintf(result,"<CR>");
				break;
			case 27:
				sprintf(result,"<ESC>");
				break;
			default:
				snprintf(result,256,"<%02hX>",c);
				break;
		}
	} else {
		snprintf(result,256,"%c",c);
	}
	} else {
	snprintf(result,256,format,c);
	}
	return result;
}

void outputchar(unsigned char c, int port, int alpha,
		long usec_threshold, long usec_waited, char *name1, char *name2, char *format)
{
	char *todisplay;

	// TODO: use switch to choose between IPC and console output
	// TODO: buffer before sending data via zmq
	send_via_zmq(c);
	
	//todisplay=chardecide(c,alpha,format);
	//disp_outputstr(port, todisplay, usec_threshold, usec_waited, name1, name2);
}

void mainloop(int port1, int port2, int silent, int alpha, int quit_on_eof, long usec_threshold, char *name1, char *name2, char *format)
{
	unsigned char c1, c2;
	int rc;
	fd_set rfds;
	fd_set efds;
	int biggestfd=0;
	int fdret;
	struct timeval before;
	struct timeval after;
	long timediff;
	int quit=0;

	// create a zmq socket for IPC
	sock.bind("tcp://127.0.0.1:5678");

	/* need the largest fd for the select call */
	biggestfd=port1 > port2 ? port1 : port2;
	biggestfd++;

	while (!quit) {
		/* reset the select set */
		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		FD_SET(port1,&rfds);
		FD_SET(port1,&efds);
		FD_SET(port2,&rfds);
		FD_SET(port2,&efds);

		if (gettimeofday(&before,NULL)) {
			perror("gettimeofday");
			exit(1);
		}
		if ((fdret=select(biggestfd, &rfds, NULL, &efds, NULL))<0) {
			perror("select");
			exit(1);
		}
		if (gettimeofday(&after,NULL)) {
			perror("gettimeofday");
			exit(1);
		}

		/* get seconds difference */
		timediff=after.tv_sec-before.tv_sec;
		/* convert to micro seconds */
		timediff*=USEC;
		/* add difference in usecs */
		timediff+=after.tv_usec-before.tv_usec;
		
		if (FD_ISSET(port1, &rfds)) {
			for (rc=read(port1, &c1, 1);
				rc>0; rc=read(port1, &c1, 1) ) {
				outputchar(c1,1,alpha,usec_threshold,timediff,name1,name2,format);
				timediff=0;
				if (!silent) write(port2,&c1,1);
			}
			if (rc==0 && quit_on_eof==1) {
				/* EOF? */
				quit=1;
			}
			if (rc<0 && errno!=EAGAIN) {
				perror("read(port1)");
				exit(1);
			}
		}
		
		if (FD_ISSET(port2, &rfds)) {
			for (rc=read(port2, &c2, 1);
				rc>0; rc=read(port2, &c2, 1) ) {
				outputchar(c2,2,alpha,usec_threshold,timediff,name1,name2,format);
				timediff=0;
				if (!silent) write(port1,&c2,1);
			}	
			if (rc==0 && quit_on_eof==1) {
				/* EOF? */
				quit=1;
			}
			if (rc<0 && errno!=EAGAIN) {
				perror("read(port2)");
				exit(1);
			}
		}

		/* check for exceptions (sockets closed, broken, etc) */
		if (FD_ISSET(port1, &efds)) {
			/* I can't remember right now what to actually
			check for on a fd exception, so we'll just quit */
			fprintf(stderr,"\nException on port1\n");
			quit=1;
		}
		if (FD_ISSET(port2, &efds)) {
			/* I can't remember right now what to actually
			check for on a fd exception, so we'll just quit */
			fprintf(stderr,"\nException on port2\n");
			quit=1;
		}
	}

	closeport(port2);
	closeport(port1);
}

void usage()
{
	fprintf(stderr,"sersniff v%s\n"

"Usage:\n"
"sersniff [-h] [-i DEV | -l PORT] [-o DEV | -c HOST:PORT] [-b BAUD] [-w USEC]\n"
"-h		This help\n"
"-x		Show hex characters instead of alpha\n"
"-f PRINTF_OPTS	printf style options for printing hex characters\n"
"		when '-x' switch is given (default \"<%%02hX>\")\n"
"-i DEVICE	Port 1 device (defaults to /dev/ttyS0). Use host:port for\n"
"                TCP.\n"
"-1 PORT1_NAME	Port 1 name to be printed (defaults to 'Port1')\n"
"-o DEVICE	Port 2 device (defaults to /dev/ttyS1). Use :port for TCP.\n"
"-2 PORT2_NAME	Port 2 name to be printed (defaults to 'Port2')\n"
"-b BAUD		Baud rate (Defaults to 19200)\n"
"-n 		No port configuration (do not set BAUD or change settings)\n"
"-w USECS	How many microsecs to wait before reporting a delay\n"
"			(default is %d)\n"
"-s 		Silent - don't pass data from port1 <=> port2,\n"
"			just display what we see from them.\n"
"-z		Don't quit when an EOF character is received.\n"
,VERSION,USEC);
	exit(1);
}

int main(int argc, char *argv[])
{
	int optret, speed;
	int port1, port2;
	char *dev1=NULL;
	char *dev2=NULL;
	char *name1=NULL;
	char *name2=NULL;
	int listenport=0;
	int connectport;
	char *connecthost=NULL, *tmpchr=NULL;
	int show_alpha=1;
	speed_t baud=B0;
	int silent=0;
	long usec_threshold=USEC;
	int setup_port=1;
	int quit_on_eof=1;
	char *format=NULL;

	while ((optret=getopt(argc,argv,"hxsnzi:l:o:c:b:w:1:2:f:"))!=EOF) {
		switch (optret) {
		case '?': case 'h': case ':':
			usage();
		case 's':
			silent=1;
			break;
		case 'w':
			usec_threshold=atoi(optarg);
			break;
		case 'i':
			if ((tmpchr=strchr(optarg, ':'))!=NULL &&
							(strchr(optarg, '/')==NULL)) {
				*tmpchr='\0';
				listenport=atoi(++tmpchr);
			} else {
				dev1=strdup(optarg);
			}
			break;
		case 'o':
			if ((tmpchr=strchr(optarg, ':'))!=NULL &&
							(strchr(optarg, '/')==NULL)) {
				*tmpchr='\0';
				connectport=atoi(++tmpchr);
				connecthost=strdup(optarg);
			} else {
				dev2=strdup(optarg);
			}
			break;
		case 'x':
			show_alpha=0;
			break;
		case 'b':
			for (speed=0;
				speed_str[speed];
				speed++) {
				if (strstr(optarg,speed_str[speed])==optarg) {
					baud=speed_num[speed];
					break;
				}
			}
			if (baud==B0) {
				fprintf(stderr,"Unsupported Baud: '%s'\n",
					optarg);
				exit(1);
			}
			break;
		case 'n':
			setup_port=0;
			break;
		case '1':
			name1=strdup(optarg);
			break;
		case '2':
			name2=strdup(optarg);
			break;
		case 'f':
			format=strdup(optarg);
			break;
		case 'z':
			quit_on_eof=0;
			break;
		}
	}

	/* Default settings */
	if (!dev1 && !listenport) dev1=strdup("/dev/ttyS0");
	if (!name1 && !listenport) name1=strdup("Port1");
	if (!dev2 && !connecthost) dev2=strdup("/dev/ttyS1");
	if (!name2 && !connecthost) name2=strdup("Port2");
	if (baud==B0) baud=B19200;
	if (!format) format=strdup("0x%02hX");

	disp_init();
	if (dev1) {
		port1=openport(dev1, baud, setup_port);
	} else {
		disp_outputstatus("Waiting for connection to TCP port.");
		port1=listensock(listenport);
	}

	if (dev2) {
		port2=openport(dev2, baud, setup_port);
	} else {
		disp_outputstatus("Connecting to TCP port.");
		port2=opensock(connecthost, connectport);
	}

	if (port1 < 0 || port2 < 0) {
		fprintf(stderr,"Argh.  An open failed!\n");
		exit(1);
	}

	mainloop(port1, port2, silent, show_alpha, quit_on_eof, usec_threshold, name1, name2, format);

	/* Clean up */
	if (dev1) free(dev1);
	if (dev2) free(dev2);
	if (connecthost) free(connecthost);
	disp_close();

	return 0;
}