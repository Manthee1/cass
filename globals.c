
#ifndef GLOBALS_C
#define GLOBALS_C
#include "headers.h"

struct FileContents contents = {0, NULL};

struct SectionIndex dataSection = {-1, -1, -1};
struct SectionIndex codeSection = {-1, -1, -1};

struct Labels labels = {0, NULL};
struct DataList dataList = {0, NULL};
struct Program program = {0, NULL};

int registers[REGISTER_COUNT] = {0};
int compareFlag = 0;
char* argTypeMap[5] = {"Register", "Number", "Label", "Data Pointer (Int)", "Data Pointer (Str)"};
enum ARG_TYPE { REGISTER, NUMBER, LABEL, DATA_POINTER_INT, DATA_POINTER_STR, UNKNOWN };
char** output = NULL;

#endif