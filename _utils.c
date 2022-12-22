
#ifndef UTILS_C
#define UTILS_C

#include "headers.h"
#include "globals.c"

void printException(char* message, enum EXCEPTION_TYPE type, int lineNum);

/**
 *@brief push a string to the end of a StringArray
 *
 * @param arr - the StringArray to push to
 * @param str - the string to push
 */
void pushString(struct StringArray* arr, char* str) {
	arr->data = realloc(arr->data, sizeof(char*) * (arr->length + 1));
	arr->data[arr->length] = str;
	arr->length++;
}

/**
 *@brief Get the index of a data value in a DataList given its name
 *
 * @param dataList - the DataList to search
 * @param name - the name of the data value to search for
 * @return int - the index of the data value in the DataList, or -1 if it doesn't exist
 */
int getDataIndex(struct DataList dataList, char* name) {
	if (dataList.data == NULL || dataList.length == 0) return -1;
	for (int i = 0; i < dataList.length; i++)
		if (strcmp(dataList.data[i].name, name) == 0) return i;
	return -1;
}

/**
 *@brief Get the Data object given its name
 *
 * @param dataList - the DataList to search
 * @param name - the name of the data value to search for
 * @return struct Data
 */
struct Data getData(struct DataList dataList, char* name) {
	int index = getDataIndex(dataList, name);
	if (index == -1) return (struct Data){-1, NULL, -1, NULL};
	return dataList.data[index];
}

/**
 *@brief Get the Data value in int form given its name
 *
 * @param dataList - the DataList to search
 * @param index - the index of the data value to search for
 * @return int - the value of the data value in int form
 */
int getDataInt(struct DataList dataList, int index) {
	if (index == -1) return 0;
	if (dataList.data[index].type == TYPE_INT) return *(int*)dataList.data[index].value;
	return 0;
}

/**
 *@brief Get the Data value in char* form given its name
 *
 * @param dataList - the DataList to search
 * @param index - the index of the data value to search for
 * @return char*
 */
char* getDataStr(struct DataList dataList, int index) {
	if (index == -1) return "";
	if (dataList.data[index].type == TYPE_STR) return (char*)dataList.data[index].value;
	return "";
}

/**
 *@brief Get the Label object given its name
 *
 * @param labels - the Labels to search
 * @param label - the name of the label to search for
 * @return struct Label
 */
struct Label getLabel(struct Labels labels, char* label) {
	if (labels.data == NULL || labels.length == 0) return (struct Label){-1, -1, NULL};
	for (int i = 0; i < labels.length; i++)
		if (strcmp(labels.data[i].name, label) == 0) return labels.data[i];
	return (struct Label){-1, -1, NULL};
}
/**
 *@brief Check if a label is on a line
 *
 * @param labels - the Labels to search
 * @param line - the line to search for
 * @return int
 */
int isLabelOnLine(struct Labels labels, int line) {
	for (int i = 0; i < labels.length; i++)
		if (labels.data[i].lineNum == line) return 1;

	return 0;
}
// Get the label position given the line number
int getLabelPosition(struct Labels labels, char* line) { return getLabel(labels, line).PC; }

/**
 *@brief Get the type of an argument
 *
 * @param dataList - the DataList to search
 * @param arg - the argument to check
 * @return enum ARG_TYPE
 */
int getArgType(struct DataList dataList, char* arg) {
	// Check if it's a register
	if (arg[0] == '$') {
		return REGISTER;
	}

	// Check if it's a number
	if (isdigit(arg[0]) || arg[0] == '-') {
		return NUMBER;
	}

	// Check if it's a label
	if (getLabel(labels, arg).lineNum != -1) return LABEL;

	// Check if it's a data pointer
	if (arg[0] == '#') {
		struct Data data = getData(dataList, &arg[1]);
		if (data.lineNum != -1) return data.type == TYPE_INT ? DATA_POINTER_INT : DATA_POINTER_STR;
		return DATA_POINTER_GENERIC;
	}

	return UNKNOWN;
}
/**
 * @brief Validate an argument (It also prints an error if it's invalid)
 *
 * @param dataList
 * @param arg
 * @param argType
 * @param lineNum
 * @return int
 */
int validateArgument(struct DataList dataList, char* arg, int argType, int lineNum) {
	switch (argType) {
		{
		case REGISTER:
			if (atoi(&arg[1]) >= registerCount) {
				printException("Invalid register", ERROR, lineNum);
				printf("\tRegister: %s does not exist (Only 0-%d)\n", arg, registerCount - 1);
				printf("\tYou can supply -r <amount> to change the amount of registers\n");
				return 1;
			}
			break;
		case NUMBER:
			if (atoi(arg) < -maxNumberSize || atoi(arg) > maxNumberSize) {
				printException("Invalid number", ERROR, lineNum);
				printf("\tNumber: %s is too big (Max: %d)\n", arg, maxNumberSize);
				printf("\tYou can supply " YELLOW "-s <size>" RESET
					   " to change the max size of a register (in bytes)\n");
				return 1;
			}
			break;
		case LABEL:
			if (getLabel(labels, arg).lineNum == -1) {
				printException("Invalid label", ERROR, lineNum);
				printf("\tLabel: %s does not exist\n", arg);
				return 1;
			}
			break;
		case DATA_POINTER_INT:
		case DATA_POINTER_STR:
		case DATA_POINTER_GENERIC:
			if (getData(dataList, &arg[1]).lineNum == -1) {
				printException("Data variable undefined", ERROR, lineNum);
				printf(YELLOW "\t\t%s" RESET " is not defined in the data section\n", &arg[1]);
				return 1;
			}
			break;
		default:
			printException("Invalid argument", ERROR, lineNum);
			return 1;
		}
	}

	return 0;
}

/**
 *@brief Convert an argument to an int
 *
 * @param dataList - the DataList to search
 * @param arg - the argument to convert
 * @param argType - the type of the argument
 * @return int
 */
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

/**
 *@brief Return a new string with the leading and trailing whitespace removed
 *
 * @param str - the string to trim
 * @return char*
 */
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

/**
 *@brief Get the amount of lines in a file
 *
 * @param file - the file to get the amount of lines from
 * @return int
 */
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

// Very similar to getFileLengthLines, which makes it kinda redundant, but it's little different and I'm too lazy.
/**
 *@brief Get the amount of characters in a file
 *
 * @param file - the file to get the amount of characters from
 * @return int
 */
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

// Wrap a number between a min and max
int wrap(int num, int min, int max) {
	if (num < min) return max;
	if (num > max) return min;
	return num;
}

void printLine(int lineNum) { printf(MAGENTA "\t%d " RESET "| %s\n", lineNum + 1, contents.data[lineNum]); }

// ------- ERROR HANDLING -------

/**
 *@brief Print an exception
 *
 * @param message - the message to print
 * @param type - the type of exception (ERROR, WARNING, INFO)
 * @param lineNum - the line number of the exception (-1 if not applicable)
 */
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