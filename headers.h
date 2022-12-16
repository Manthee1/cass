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
struct SectionIndex {
	int length;
	int start;
	int end;
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

enum DATA_TYPE { TYPE_INT, TYPE_STR };
struct Data {
	int lineNum;
	char* name;
	enum DATA_TYPE type;
	void* value;  // Pointer to the value
};

struct DataList {
	int length;
	struct Data* data;
};

struct ProgramInstruction {
	int lineNum;
	int* args;
	int instructionIndex;  // A pointer would take up double the space. so we use an index instead
	int dataIndex;
};

struct Program {
	int length;
	struct ProgramInstruction* instructions;
};

#define REGISTER_COUNT 16
#define REGISTER_SIZE 32
#define MAX_NUMBER_SIZE (int)pow(2, REGISTER_SIZE)

int registers[REGISTER_COUNT] = {0};
int compareFlag = 0;
char* argTypeMap[5] = {"Register", "Number", "Label", "Data Pointer (Int)", "Data Pointer (Str)"};
enum ARG_TYPE { REGISTER, NUMBER, LABEL, DATA_POINTER_INT, DATA_POINTER_STR, UNKNOWN };
char** output = NULL;
int outputLines = 0;
void addToOutput(char* str) {
	outputLines++;
	output = realloc(output, outputLines * sizeof(char*));
	output[outputLines - 1] = malloc(sizeof(char) * strlen(str));
	strcpy(output[outputLines - 1], str);
}

int getDataIndex(struct DataList dataList, char* name) {
	if (dataList.data == NULL || dataList.length == 0) return -1;
	for (int i = 0; i < dataList.length; i++)
		if (strcmp(dataList.data[i].name, name) == 0) return i;
	return -1;
}
struct Data getData(struct DataList dataList, char* name) {
	int index = getDataIndex(dataList, name);
	if (index == -1) return (struct Data){-1, NULL, -1, NULL};
	return dataList.data[index];
}

int getDataInt(struct DataList dataList, int index) {
	if (index == -1) return 0;
	if (dataList.data[index].type == TYPE_INT) return *(int*)dataList.data[index].value;
	return 0;
}

char* getDataStr(struct DataList dataList, int index) {
	if (index == -1) return "";
	if (dataList.data[index].type == TYPE_STR) return (char*)dataList.data[index].value;
	return "";
}

struct Label getLabel(struct Labels labels, char* label) {
	if (labels.data == NULL || labels.length == 0) return (struct Label){-1, -1, NULL};
	for (int i = 0; i < labels.length; i++)
		if (strcmp(labels.data[i].name, label) == 0) return labels.data[i];
	return (struct Label){-1, -1, NULL};
}
int isLabelOnLine(struct Labels labels, int line) {
	for (int i = 0; i < labels.length; i++)
		if (labels.data[i].lineNum == line) return 1;

	return 0;
}
int getLabelPosition(struct Labels labels, char* line) { return getLabel(labels, line).PC; }

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

int isCommentOnLine(struct FileContents contents, int line) {
	if (line >= contents.length) return 0;
	return isComment(contents.data[line]);
}
