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
struct Labels {
	int length;
	struct Label* data;
};

struct Instruction {
	char* name;
	int argCount;
	int* argTypes;
	void (*func)(int*, int, char**);
};

struct FileContents text = {0, NULL};
struct FileContents data = {0, NULL};
struct Labels labels = {0, NULL};

#define REGISTER_COUNT 16
#define REGISTER_SIZE 8
#define MAX_NUMBER_SIZE pow(2, REGISTER_SIZE)

int registers[REGISTER_COUNT] = {0};
int compareFlag = 0;
char* argTypeMap[5] = {"Register", "Number", "Label", "Data Pointer", "Unknown"};

int getGlobalLineNum(int line) { return line + text.length + 3; }
int isLabel(int line) {
	for (int i = 0; i < labels.length; i++) {
		if (labels.data[i].position == line) return 1;
	}
	return 0;
}
struct Label getLabel(char* label) {
	if (labels.data == NULL || labels.length == 0) return (struct Label){-1, NULL};
	for (int i = 0; i < labels.length; i++)
		if (strcmp(labels.data[i].name, label) == 0) return labels.data[i];
	return (struct Label){-1, NULL};
}
int getLabelPosition(char* label) { return getLabel(label).position; }
void printLine(int lineNum) {
	char* line = data.data[lineNum];
	printf(MAGENTA "\t%d " RESET "| %s", getGlobalLineNum(lineNum), data.data[lineNum]);
}