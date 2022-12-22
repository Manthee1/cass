
#ifndef HEADERS_H
#define HEADERS_H

#include "ctype.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "time.h"

#define BOLD "\033[1m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET "\033[0m"

// Holds only the text of a file
struct FileContents {
	int length;
	// Data is a array of pointers to each line
	char** data;
};

// start and end of a "section" in a file (just a line range)
struct SectionIndex {
	int length;
	int start;
	int end;
};

// Label position, name, and line number
struct Label {
	int lineNum;
	int PC;
	char* name;
};

// A list of labels
struct Labels {
	int length;
	struct Label* data;
};

// Used in instructions.c to store the name, argument count, argument types, and a pointer to the instruction function
struct Instruction {
	char* name;
	int argCount;
	int* argTypes;
	void (*func)(int*, ...);
};

enum DATA_TYPE { TYPE_INT, TYPE_STR };
// A data value, type, name, and line number
struct Data {
	int lineNum;
	char* name;
	enum DATA_TYPE type;
	void* value;  // Pointer to the value because it can be either an int or a char*
};

// A list of data values
struct DataList {
	int length;
	struct Data* data;
};

// A program instruction, line number, and a pointer to the instruction
struct ProgramInstruction {
	int lineNum;
	int* args;
	int instructionIndex;  // A pointer would take up double the space. so we use an index instead
};

// A list of all program instructions
struct Program {
	int length;
	struct ProgramInstruction* instructions;
};

// A list of strings
struct StringArray {
	int length;
	char** data;
};

enum EXCEPTION_TYPE { ERROR, WARNING, INFO };

#define REGISTER_COUNT 16
#define REGISTER_SIZE 32
#define MAX_NUMBER_SIZE (int)pow(2, REGISTER_SIZE)

#endif