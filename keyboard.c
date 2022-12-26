#include "stdio.h"
#include "termios.h"
#include "unistd.h"
#include "sys/select.h"
#include "sys/time.h"
#include "sys/ioctl.h"

static struct termios oldt, newt;

int kInit() {
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

int parseKeyPress(char* buffer) {
	if (buffer[0] != 27) return buffer[0];
	if (buffer[1] == 0) return 27;
	printf("\b \b");

	// If the rest starts with a [ it's a special key
	if (buffer[1] != '[') return -1;
	// If it doesn't have a second character just return the value of the first
	if (buffer[3] == 0) return 200 + buffer[2];

	// If it ends with a ~ Return the number after the [ and before the ~
	if (buffer[strlen(buffer) - 1] == '~') {
		buffer[strlen(buffer) - 1] = 0;
		// Take it's number and return it
		int num = 0;
		sscanf(buffer + 2, "%d", &num);
		return 200 + num;
	}

	return -1;
}
int getKeyPress() {
	char buffer[10] = {0};

	// Get length of stdin
	int len = 0;
	ioctl(STDIN_FILENO, FIONREAD, &len);

	if (len < 1) return -1;
	for (int i = 0; i < len; i++) buffer[i] = getchar();

	// Remove inputted characters
	for (int i = 0; i < len; i++) printf("\b \b");

	return parseKeyPress(buffer);
}

void kEnd() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); }