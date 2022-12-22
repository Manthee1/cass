
#ifndef GLOBALS_C
#define GLOBALS_C
#include "headers.h"

#define DEFAULT_REGISTER_COUNT 16
#define DEFAULT_REGISTER_SIZE 32

// Initialize all structs and needed arrays into global variables
struct FileContents contents = {0, NULL};

struct SectionIndex dataSection = {-1, -1, -1};
struct SectionIndex codeSection = {-1, -1, -1};

struct Labels labels = {0, NULL};
struct DataList dataList = {0, NULL};
struct Program program = {0, NULL};

int registerCount = DEFAULT_REGISTER_COUNT;
int registerSize = DEFAULT_REGISTER_COUNT;
int* registers = NULL;	// Initialized in main()
int maxNumberSize = 0;	// Initialized in main()
int compareFlag = 0;
char* argTypeMap[5] = {"Register", "Number", "Label", "Data Pointer (Int)", "Data Pointer (Str)"};
enum ARG_TYPE { REGISTER, NUMBER, LABEL, DATA_POINTER_INT, DATA_POINTER_STR, UNKNOWN };
struct StringArray output = {0, NULL};

#endif