#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define VERSION "0.0.5"
#include "disp_basic.h"
#include "tcp.h"
#include "sersniff.h"

char * speed_str[] = { "300", "1200", "2400", "4800", "9600", "19200", "38400",
		"57600", "115200", "230400", NULL };
speed_t speed_num[] = { B300, B1200, B2400, B4800, B9600, B19200, B38400,
		B57600, B115200, B230400, B0 };

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
"-o DEVICE	Port 2 device (defaults to /dev/ttyS1). Use :port for TCP.\n"
"-b BAUD		Baud rate (Defaults to 19200)\n"
"-a      		Log the incoming traffic to console\n"
"-n 		No port configuration (do not set BAUD or change settings)\n"
"-w USECS	How many microsecs to wait before reporting a delay\n"
"			(default is %d)\n"
"-s 		Silent - don't pass data from port1 <=> port2,\n"
"			just display what we see from them.\n"
"-z		Don't quit when an EOF character is received.\n"
,VERSION,USEC);
	::exit(1);
}

int main(int argc, char *argv[])
{
	int optret, speed;
	int port1, port2;
	char *dev1=NULL;
	char *dev2=NULL;
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
	bool logToConsole = false;

	while ((optret=getopt(argc,argv,"ahxsnzi:l:o:c:b:w:1:2:f:"))!=EOF) {
		switch (optret) {
		case '?': case 'h': case ':':
			usage();
		case 'a':
			logToConsole = true;
			break;
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
				::exit(1);
			}
			break;
		case 'n':
			setup_port=0;
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
	//if (!dev1 && !listenport) dev1=strdup("/dev/ttyS0");
	//if (!name1 && !listenport) name1=strdup("Port1");
	//if (!dev2 && !connecthost) dev2=strdup("/dev/ttyS1");
	//if (!name2 && !connecthost) name2=strdup("Port2");
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
	} else if (connecthost) {
		disp_outputstatus("Connecting to TCP port.");
		port2=opensock(connecthost, connectport);
	}

	if (port1 < 0 || port2 < 0) {
		fprintf(stderr,"Argh.  An open failed!\n");
		::exit(1);
	}

	mainloop(port1, port2, silent, show_alpha, quit_on_eof, usec_threshold, format, logToConsole);

	/* Clean up */
	if (dev1) free(dev1);
	if (dev2) free(dev2);
	if (connecthost) free(connecthost);
	disp_close();

	return 0;
}