
#ifndef UTILS_C
#define UTILS_C

#include "headers.h"
#include "globals.c"

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

int getArgType(struct DataList dataList, char* arg) {
	// Check if it's a register
	if (arg[0] == '$') {
		int reg = atoi(&arg[1]);
		if (reg >= 0 && reg < REGISTER_COUNT) return REGISTER;
	}

	// Check if it's a number
	if (isdigit(arg[0]) || arg[0] == '-') {
		int num = atoi(arg);
		if (num >= -MAX_NUMBER_SIZE && num <= MAX_NUMBER_SIZE) return NUMBER;
	}

	// Check if it's a label
	if (getLabel(labels, arg).lineNum != -1) return LABEL;

	// Check if it's a data pointer
	if (arg[0] == '#') {
		struct Data data = getData(dataList, &arg[1]);
		if (data.lineNum != -1) return data.type == TYPE_INT ? DATA_POINTER_INT : DATA_POINTER_STR;
	}

	return UNKNOWN;
}

int convertArg(struct DataList dataList, char* arg, int argType) {
	int val = -1;
	switch (argType) {
		{
		case REGISTER:
			sscanf(arg, "$%d", &val);
			break;
		case NUMBER:
			return atoi(arg);
		case LABEL:
			return getLabel(labels, arg).PC;
		case DATA_POINTER_INT:
		case DATA_POINTER_STR:
			val = 0;  // This is a hack to make the IDE happy
			// Get the data name
			val = getDataIndex(dataList, &arg[1]);
			break;
		default:
			val = -1;
			break;
		}
	}
	return val;
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

int isCommentOnLine(struct FileContents contents, int line) {
	if (line >= contents.length) return 0;
	return isComment(contents.data[line]);
}

void exitMsg(char* message, int code) {
	printf("%s\n", message);
	exit(code);
}

void clear() { system("clear || cls"); }

void toLowerStr(char* str) {
	for (int i = 0; i < strlen(str); i++) str[i] = (str[i] >= 'A' && str[i] <= 'Z') ? str[i] + 32 : str[i];
}

char* trimStr(char* str) {
	int start = 0;
	int end = strlen(str) - 1;
	while (str[start] == ' ' || str[start] == '\t') start++;
	while (str[end] == ' ' || str[end] == '\t') end--;
	char* newStr = malloc(sizeof(char) * (end - start + 2));
	for (int i = start; i <= end; i++) newStr[i - start] = str[i];
	newStr[end - start + 1] = '\0';
	return newStr;
}

int getFileLengthLines(FILE* file) {
	// Save pointer
	fpos_t pos;
	fgetpos(file, &pos);

	// Get amount of lines
	int length = 0;
	char c;
	while ((c = fgetc(file)) != EOF)
		if (c == '\n') length++;

	// Reset pointer
	fsetpos(file, &pos);

	return length;
}

int getFileLength(FILE* file) {
	// Save pointer
	fpos_t pos;
	fgetpos(file, &pos);

	// Get amount of characters
	fseek(file, 0, SEEK_END);
	int length = ftell(file);

	// Reset pointer
	fsetpos(file, &pos);

	return length;
}

int wrap(int num, int min, int max) {
	if (num < min) return max;
	if (num > max) return min;
	return num;
}

void printLine(int lineNum) { printf(MAGENTA "\t%d " RESET "| %s\n", lineNum + 1, contents.data[lineNum]); }

// ------- ERROR HANDLING -------

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
	printf(" on line " MAGENTA "%d" RESET ": %s\n", lineNum + 1, message);
	printLine(lineNum);
}

#endif