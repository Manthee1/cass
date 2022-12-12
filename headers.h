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
	int lineNum;
	int PC;
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
	void (*func)(int*, ...);
};

struct ProgramInstruction {
	int lineNum;
	int* args;
	int instructionIndex;  // A pointer would take up double the space. so we use an index instead
};

struct Program {
	int length;
	struct ProgramInstruction* instructions;
};

struct FileContents text = {0, NULL};
struct FileContents data = {0, NULL};
struct Labels labels = {0, NULL};
struct Program program = {0, NULL};
#define REGISTER_COUNT 16
#define REGISTER_SIZE 32
#define MAX_NUMBER_SIZE (int)pow(2, REGISTER_SIZE)

int registers[REGISTER_COUNT] = {0};
int compareFlag = 0;
char* argTypeMap[5] = {"Register", "Number", "Label", "Data Pointer", "Unknown"};
enum ARG_TYPE { REGISTER, NUMBER, LABEL, DATA_POINTER, UNKNOWN };
char** output = NULL;
int outputLines = 0;
void addToOutput(char* str) {
	outputLines++;
	output = realloc(output, outputLines * sizeof(char*));
	output[outputLines - 1] = malloc(sizeof(char) * strlen(str));
	strcpy(output[outputLines - 1], str);
}

int getGlobalLineNum(int line) { return line + text.length + 3; }
int isLabelOnLine(int line) {
	for (int i = 0; i < labels.length; i++)
		if (labels.data[i].lineNum == line) return 1;

	return 0;
}
struct Label getLabel(char* label) {
	if (labels.data == NULL || labels.length == 0) return (struct Label){-1, -1, NULL};
	for (int i = 0; i < labels.length; i++)
		if (strcmp(labels.data[i].name, label) == 0) return labels.data[i];
	return (struct Label){-1, -1, NULL};
}
int getLabelPosition(char* label) { return getLabel(label).PC; }
void printLine(int lineNum) {
	char* line = data.data[lineNum];
	printf(MAGENTA "\t%d " RESET "| %s\n", getGlobalLineNum(lineNum), data.data[lineNum]);
}

int isLabel(char* str) {
	int strLen = strlen(str);
	if (strLen == 0) return 0;
	if (str[strLen - 1] == ':') return 1;
	return 0;
}

int isComment(char* str) {
	if (str[0] == '#' || str[0] == ';') return 1;
	return 0;
}

int isCommentOnLine(int line) {
	if (line >= data.length) return 0;
	return isComment(data.data[line]);
}

// ------- ERROR HANDLING -------
enum EXCEPTION_TYPE { ERROR, WARNING, INFO };

void printException(char* message, enum EXCEPTION_TYPE type, int lineNum) {
	switch (type) {
	case ERROR:
		printf(RED "Error" RESET);
		break;
	case WARNING:
		printf(YELLOW "Warning" RESET);
		break;
	case INFO:
		printf(GREEN "Info" RESET);
		break;
	}
	if (lineNum == -1) {
		printf(": %s\n", message);
		return;
	}
	printf(" on line " MAGENTA "%d" RESET ": %s\n", getGlobalLineNum(lineNum), message);
	printLine(lineNum);
}
