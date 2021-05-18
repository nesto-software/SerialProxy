#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define VERSION "0.0.5"
#include "greengrasssdk.h"
#include "../disp_basic.h"
#include "../sersniff.h"

char * speed_str[] = { "300", "1200", "2400", "4800", "9600", "19200", "38400",
		"57600", "115200", "230400", NULL };
speed_t speed_num[] = { B300, B1200, B2400, B4800, B9600, B19200, B38400,
		B57600, B115200, B230400, B0 };

void handler(const gg_lambda_context *cxt) {
    (void)cxt;
    return;
}

int main() {
    gg_error err = GGE_SUCCESS;

    err = gg_global_init(0);
    if(err) {
        gg_log(GG_LOG_ERROR, "gg_global_init failed %d", err);
        return -1;
    }

    gg_runtime_start(handler, GG_RT_OPT_ASYNC);

    const char* baudRate = std::getenv("BAUD_RATE");
	char* dev1 = std::getenv("DEV_1");
	char* dev2 = std::getenv("DEV_2");

	int speed;
	int port1, port2;
	int show_alpha=1;
	speed_t baud=B0;
	int silent=0;
	long usec_threshold=USEC;
	int setup_port=1;
	char *format=strdup("0x%02hX");
	bool logToConsole = false;
	int quit_on_eof=0;
	
	// set baud rate
	for (speed=0;
		speed_str[speed];
		speed++) {
		if (strstr(baudRate,speed_str[speed])==baudRate) {
			baud=speed_num[speed];
			break;
		}
	}
	if (baud==B0) {
		fprintf(stderr,"Unsupported Baud: '%s'\n",
			baudRate);
		::exit(1);
	}

	disp_init();
    port1=openport(dev1, baud, setup_port);
    port2=openport(dev2, baud, setup_port);

	mainloop(port1, port2, silent, show_alpha, quit_on_eof, usec_threshold, format, logToConsole);

	/* Clean up */
	if (dev1) free(dev1);
	if (dev2) free(dev2);
	disp_close();

	return -1;
}