#include "termios.h"
#include "unistd.h"
#include "sys/select.h"
#include "sys/time.h"

static struct termios oldt, newt;

int kInit() {
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void kEnd() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); }