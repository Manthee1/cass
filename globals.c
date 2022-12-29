
#ifndef GLOBALS_C
#define GLOBALS_C
#include "headers.h"

#define VERSION "0.2.0"

#define DEFAULT_REGISTER_COUNT 16
#define DEFAULT_REGISTER_SIZE 32
#define MAX_REGISTER_COUNT 64
#define MAX_REGISTER_SIZE 32
#define MAX_SPEED 1000

// Initialize all structs and needed arrays into global variables
struct FileContents contents = {0, NULL};

struct SectionIndex dataSection = {-1, -1, -1};
struct SectionIndex codeSection = {-1, -1, -1};

struct Labels labels = {0, NULL};
struct DataList dataList = {0, NULL};
struct Program program = {0, NULL};

// Options
int debug = 0;
int verbose = 0;
int strict = 0;
int speed = 1;
int registerCount = DEFAULT_REGISTER_COUNT;
int registerSize = DEFAULT_REGISTER_COUNT;

// Screen size
int screenWidth = 0;
int screenHeight = 0;

// Debug
int paused = 0;

int* registers = NULL;		  // Initialized in main()
long long maxNumberSize = 0;  // Initialized in main()
int compareFlag = 0;
char* argTypeMap[6] = {
	"Register", "Number", "Label", "Data Pointer (Int)", "Data Pointer (Str)", "Data Pointer (Generic)"};
enum ARG_TYPE { REGISTER, NUMBER, LABEL, DATA_POINTER_INT, DATA_POINTER_STR, DATA_POINTER_GENERIC, UNKNOWN };
struct StringArray output = {0, NULL};

#endif