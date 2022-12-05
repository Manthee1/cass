#include "./_utils.c"
#include "ctype.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#define BOLD "\033[1m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET "\033[0m"

struct FileContents {
	int length;
	// Data is a array of pointers to each line
	char** data;
};
struct Label {
	int position;
	char* name;
};

struct Instruction {
	char* name;
	int argCount;
	int* argTypes;
	void (*func)(int*, int, char**);
};

struct FileContents text = {0, NULL};
struct FileContents data = {0, NULL};
int labelCount = 0;
struct Label* labels = NULL;

#define REGISTER_COUNT 16

int registers[REGISTER_COUNT] = {0};

int getGlobalLineNum(int line) { return line + text.length + 3; }
int isLabel(int line) {
	for (int i = 0; i < labelCount; i++)
		if (labels[i].position == line) return 1;
	return 0;
}
int getLabelPosition(char* label) {
	for (int i = 0; i < labelCount; i++)
		if (strcmp(labels[i].name, label) == 0) return labels[i].position;
	return -1;
}

void printLine(int lineNum) {
	char* line = data.data[lineNum];
	printf(MAGENTA "\t%d " RESET "| %s", getGlobalLineNum(lineNum), data.data[lineNum]);
}