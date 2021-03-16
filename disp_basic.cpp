/*
	disp_basic.c - Routines to do basic output with no fanciness.
	Copyright 2000 Project Purple. Written by Jonathan McDowell

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

	17/02/2000 - Started writing.
*/

#include <stdio.h>
#include <string.h>

#include "disp_basic.h"

/* Which port did we display last? */
static int last=-1;
/* Count of chars so far this line */
static int wrap=WRAPINIT;


/* Initialise the display, ie do nothing */
int disp_init()
{
	return 0;
}

/* Close the display, ie do nothing */
int disp_close()
{
	return 0;
}

/* Output a status string about what's happening */
void disp_outputstatus(char *string)
{
	printf("\n%s\n\t", string);
	last=-1;
	wrap=WRAPINIT;
}

/* Output a string from the port. */
void disp_outputstr(int port, char *string,
		long usec_threshold, long usec_waited)
{
	if (usec_waited>usec_threshold) {
		/* report how long we waited between the last port */
		if (wrap!=WRAPINIT) printf("\n");
		printf("\t<waited %0ld.%0ld seconds>\n",
			usec_waited / USEC,
			usec_waited % USEC);
		wrap=WRAPINIT;
	}

	if (last!=port) {
		/* If we didn't just send a CR, we need to now */
		if (wrap!=WRAPINIT) printf("\n");
		//printf("\nPort%d:\t", port);
		last=port;
		wrap=WRAPINIT;
	} else if (wrap==WRAPINIT) {
		/* We should indent since we're continuing the same port
			as last time */
		printf("\t");
	}

	/* If we'd go over the line end, wrap */
	if (strlen(string)+wrap > SCREEN_WIDTH) {
		wrap=WRAPINIT;
		printf("\n\t");
	}

	wrap+=strlen(string);
	printf("%s", string);

	if (wrap % SCREEN_WIDTH == 0) {
		/* we hit the edge of the screen */
		wrap=WRAPINIT;
		printf("\n");
	}
	
	/* flush all the time */
	fflush(NULL);
}
