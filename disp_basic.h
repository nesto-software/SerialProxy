/*
	disp_basic.h - Routines to do basic output with no fanciness.
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

/* where to start wrap counts (needs to include "PortX:\t") */
#define WRAPINIT 9
/* Screen width */
#define SCREEN_WIDTH 80
#define USEC 1000000

/* Initialise the display, ie do nothing */
int disp_init();

/* Close the display, ie do nothing */
int disp_close();

/* Output a status string about what's happening */
void disp_outputstatus(char *string);

/* Output a string from the port. */
void disp_outputstr(int port, char *string,
		long usec_threshold, long usec_waited, char *name1, char *name2);

