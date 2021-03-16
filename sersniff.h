#include <termios.h>

int openport(const char *device, speed_t baud, int setup);
void mainloop(int port1, int port2, int silent, int alpha, int quit_on_eof, long usec_threshold, char *format, bool logToConsole);