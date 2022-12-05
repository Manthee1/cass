
#include "headers.h"

void movFunc(int* PC, int argCount, char* args[]) {
	int reg1 = args[0][1] - '0';
	int reg2 = args[1][1] - '0';
	registers[reg1] = registers[reg2];
}

struct Instruction mov = {"mov", 2, (int[]){0, 0}, (void*)movFunc};

/// Move constant to register
void movcFunc(int* PC, int argCount, char* args[]) {
	int reg = args[0][1] - '0';
	int value = atoi(args[1]);
	registers[reg] = value;
}

struct Instruction movc = {"movc", 2, (int[]){0, 1}, (void*)movcFunc};

// OUT
void outFunc(int* PC, int argCount, char* args[]) {
	int outputTo = atoi(args[0]);
	int reg = args[1][1] - '0';
	switch (outputTo) {
	case 0:
		printf("%d", registers[reg]);
		break;

	default:
		break;
	}
}
struct Instruction out = {"out", 2, (int[]){1, 0}, (void*)outFunc};

// JMP
void jmpFunc(int* PC, int argCount, char* args[]) {
	int label = getLabelPosition(args[0]);
	*PC = label;
}
struct Instruction jmp = {"jmp", 1, (int[]){2}, (void*)jmpFunc};

// ADD
void addFunc(int* PC, int argCount, char* args[]) {
	int reg1 = args[0][1] - '0';
	int reg2 = args[1][1] - '0';
	// Check if the register overflows
	if (registers[reg1] + registers[reg2] > 255) {
		registers[reg1] = 255;
		printf(YELLOW "Warning:" RESET " Register " YELLOW "$%d" RESET " overflowed at " MAGENTA "line %d" RESET "\n\n",
			   reg1, getGlobalLineNum(*PC));
		return;
	}

	registers[reg1] += registers[reg2];
}
struct Instruction add = {"add", 2, (int[]){0, 0}, (void*)addFunc};

// IN
void inFunc(int* PC, int argCount, char* args[]) {
	int inputFrom = atoi(args[0]);
	int reg = args[1][1] - '0';
	switch (inputFrom) {
	case 0:
		scanf("%d", &registers[reg]);
		break;

	default:
		break;
	}
}
struct Instruction in = {"in", 2, (int[]){1, 0}, (void*)inFunc};

#define MAX_INSTRUCTION_LENGTH 5
#define INSTRUCTION_COUNT 6
void* instructions[INSTRUCTION_COUNT] = {&mov, &movc, &out, &jmp, &add, &in};