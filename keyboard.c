#include "stdio.h"
#include "termios.h"
#include "unistd.h"
#include "sys/select.h"
#include "sys/time.h"
#include "sys/ioctl.h"

static struct termios oldt, newt;

struct Keymap {
	char* keyChar;
	int key;
};

struct Keymap keymap[] = {
	{"\033", 27},	  {"\033[A", 0},	{"\033[B", 1},	  {"\033[C", 2},	{"\033[D", 3},	  {"\033[2~", 4},
	{"\033[3~", 5},	  {"\033[4~", 6},	{"\033[5~", 7},	  {"\033[6~", 8},	{"\033[7~", 9},	  {"\033[8~", 10},
	{"\033[9~", 11},  {"\033[10~", 12}, {"\033[11~", 13}, {"\033[12~", 14}, {"\033[13~", 15}, {"\033[14~", 16},
	{"\033[15~", 17}, {"\033[17~", 18}, {"\033[18~", 19}, {"\033[19~", 20}, {"\033[20~", 21}, {"\033[21~", 22},
	{"\033[23~", 23}, {"\033[24~", 24}, {"\033[25~", 25}, {"\033[26~", 26}, {"\033[28~", 27}, {"\033[29~", 28},
	{"\033[31~", 29}, {"\033[32~", 30}, {"\033[33~", 31}, {"\033[34~", 32}, {"\033[35~", 33}, {"\033[36~", 34},
	{"\033[37~", 35}, {"\033[38~", 36}, {"\033[39~", 37}, {"\033[40~", 38}, {"\033[41~", 39}, {"\033[42~", 40},
	{"\033[43~", 41}, {"\033[44~", 42}, {"\033[45~", 43}, {"\033[46~", 44}, {"\033[47~", 45}, {"\033[48~", 46},
	{"\033[49~", 47}, {"\033[50~", 48},
};

int kInit() {
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

int parseKeyPress(char* buffer) {
	if (buffer[0] != 27) return buffer[0];
	printf("\b \b");
	for (int i = 0; i < sizeof(keymap) / sizeof(struct Keymap); i++) {
		if (strcmp(buffer, keymap[i].keyChar) != 0) continue;
		printf("Found key: %d\n", keymap[i].key);
		return keymap[i].key;
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

	// Remove inputed characters
	for (int i = 0; i < len; i++) printf("\b \b");

	return parseKeyPress(buffer);
}

void kEnd() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); }