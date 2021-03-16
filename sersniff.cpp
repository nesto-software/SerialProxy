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
#include <sys/ioctl.h>
#include <thread>
#include <atomic>
#include <exception>

#include "disp_basic.h"
#include "tcp.h"
using namespace std;

zmq::context_t *ctx = new zmq::context_t();
zmq::socket_t sock (*ctx, zmq::socket_type::pub);

class ProxyException: public exception {
	std::string m_msg;
	private:
		bool isInputPort;
	public:
		ProxyException(const bool isInputPort)
			: m_msg(std::string("Port ") + ((isInputPort) ? "1" : "2") + " was probably closed.")
		{
			this->isInputPort = isInputPort;
		}

		virtual const char* what() const throw()
		{
			return m_msg.c_str();
		}

	public:
		bool checkIfInputPortMisbehaving() {
			return this->isInputPort;
		}
};


int setRTS(int fd, int level) {
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("setRTS(): TIOCMGET");
        return 0;
    }
    if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}

int setDTR(int fd, int level) {
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("setDTR(): TIOCMGET");

        throw ProxyException(0);
    }

    if (level) {
        status |= TIOCM_DTR;
	}
    else {
        status &= ~TIOCM_DTR;
	}

    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setDTR(): TIOCMSET");
        
		throw ProxyException(0);
    }
	
    return 1;
}

int getRTS(int fd) {
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("setRTS(): TIOCMGET");

        throw ProxyException(1);
    }
    
	return (status & TIOCM_CTS) != 0;
}

void handle_flow_control(int port1, int port2, std::atomic<bool>& stop_flag) {

	while (!stop_flag.load()) {
		try {
			int rts = getRTS(port1);
			setRTS(port2, rts);
			setDTR(port2, rts);

			// uncomment the following to see rts status on console
			//printf("status: %d\n", status);
			//fflush(NULL);
		} catch (ProxyException &e) {
			fprintf(stderr, "Exception: %s\n", e.what());
			fprintf(stderr, "We sleep for 2s and then exit the application.\n");

			if (e.checkIfInputPortMisbehaving()) {
				try {
					setRTS(port2, 0);
					setDTR(port2, 0);
				} catch(exception& e) {};
			}

			sleep(2);
			::exit(1);
		}

		// wait one second
		sleep(1);
	}
}

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
		fprintf(stderr, "We wait 2s before exiting.\n");
		sleep(2);

		::exit(1);
	}

	if (setup) {
		bzero(&serparams, sizeof(serparams));

		cfsetspeed(&serparams, baud);
		serparams.c_cflag |= CLOCAL | CS8 | CREAD;

		if (tcflush(filedes, TCIFLUSH)) {
			fprintf(stderr,"%s: ",device);
			perror("tcflush");
			::exit(1);
		}
		if (tcsetattr(filedes, TCSANOW, &serparams)) {
			fprintf(stderr,"%s: ",device);
			perror("tcsetattr");
			::exit(1);
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
		long usec_threshold, long usec_waited, char *format, bool logToConsole)
{
	// TODO: buffer before sending data via zmq
	// TODO: add support for CRTSCTS, see: termios.h
	send_via_zmq(c);
	
	if (logToConsole) {
		char *todisplay;
		todisplay=chardecide(c,alpha,format);
		disp_outputstr(port, todisplay, usec_threshold, usec_waited);
	}
}

void mainloop(int port1, int port2, int silent, int alpha, int quit_on_eof, long usec_threshold, char *format, bool logToConsole)
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

	std::atomic<bool> stop_flag(false);
	std::thread flow_control_thread(handle_flow_control, port1, port2, std::ref(stop_flag));

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
			::exit(1);
		}
		if ((fdret=select(biggestfd, &rfds, NULL, &efds, NULL))<0) {
			perror("select");
			::exit(1);
		}
		if (gettimeofday(&after,NULL)) {
			perror("gettimeofday");
			::exit(1);
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
				outputchar(c1,1,alpha,usec_threshold,timediff,format,logToConsole);
				timediff=0;
				if (!silent) write(port2,&c1,1);
			}
			if (rc==0 && quit_on_eof==1) {
				/* EOF? */
				quit=1;
			}
			if (rc<0 && errno!=EAGAIN) {
				perror("read(port1)");
				::exit(1);
			}
		}
		
		if (FD_ISSET(port2, &rfds)) {
			for (rc=read(port2, &c2, 1);
				rc>0; rc=read(port2, &c2, 1) ) {
				outputchar(c2,2,alpha,usec_threshold,timediff,format,logToConsole);
				timediff=0;
				if (!silent) write(port1,&c2,1);
			}	
			if (rc==0 && quit_on_eof==1) {
				/* EOF? */
				quit=1;
			}
			if (rc<0 && errno!=EAGAIN) {
				perror("read(port2)");
				::exit(1);
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

	stop_flag = true;
	flow_control_thread.join();
	closeport(port2);
	closeport(port1);
}

