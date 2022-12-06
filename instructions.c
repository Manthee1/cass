
#include "headers.h"

// ========================= INSTRUCTIONS =========================

// -----------------MOV-----------------
void movFunc(int* PC, int argCount, char* args[]) {
	int reg1 = args[0][1] - '0';
	int reg2 = args[1][1] - '0';
	registers[reg1] = registers[reg2];
}
struct Instruction mov = {"mov", 2, (int[]){0, 0}, (void*)movFunc};

// -----------------MOVC-----------------
void movcFunc(int* PC, int argCount, char* args[]) {
	int reg = args[0][1] - '0';
	int value = atoi(args[1]);
	registers[reg] = value;
}
struct Instruction movc = {"movc", 2, (int[]){0, 1}, (void*)movcFunc};

// -----------------JMP-----------------
void jmpFunc(int* PC, int argCount, char* args[]) {
	int label = getLabelPosition(args[0]);
	*PC = label;
}
struct Instruction jmp = {"jmp", 1, (int[]){2}, (void*)jmpFunc};

// -----------------JC-----------------
void jcFunc(int* PC, int argCount, char* args[]) {
	int label = getLabelPosition(args[0]);
	if (registers[0] == 0) *PC = label;
}
struct Instruction jc = {"jc", 1, (int[]){2}, (void*)jcFunc};

// -----------------CMP-----------------
void cmpFunc(int* PC, int argCount, char* args[]) {
	int reg1 = args[0][1] - '0';
	int reg2 = args[1][1] - '0';
	if (registers[reg1] == registers[reg2])
		registers[0] = 0;
	else if (registers[reg1] > registers[reg2])
		registers[0] = 1;
	else
		registers[0] = -1;
}
struct Instruction cmp = {"cmp", 2, (int[]){0, 0}, (void*)cmpFunc};

// -----------------ADD-----------------
void addFunc(int* PC, int argCount, char* args[]) {
	int reg1 = args[0][1] - '0';
	int reg2 = args[1][1] - '0';
	int sum = registers[reg1] + registers[reg2];
	// Check if the register overflows
	if (sum > MAX_NUMBER_SIZE || sum < -MAX_NUMBER_SIZE) {
		registers[reg1] = wrap(sum, MAX_NUMBER_SIZE, -MAX_NUMBER_SIZE);
		printf(YELLOW "Warning:" RESET " Register " YELLOW "$%d" RESET " overflowed at " MAGENTA "line %d" RESET "\n\n",
			   reg1, getGlobalLineNum(*PC));
		return;
	}
	registers[reg1] = sum;
}
struct Instruction add = {"add", 2, (int[]){0, 0}, (void*)addFunc};

// -----------------OUT-----------------
void outFunc(int* PC, int argCount, char* args[]) {
	int reg = args[0][1] - '0';
	printf("%d", registers[reg]);
}
struct Instruction out = {"out", 1, (int[]){0}, (void*)outFunc};

// -----------------IN-----------------
void inFunc(int* PC, int argCount, char* args[]) {
	int reg = args[0][1] - '0';
	scanf("%d", &registers[reg]);
}
struct Instruction in = {"in", 1, (int[]){0}, (void*)inFunc};

#define MAX_INSTRUCTION_LENGTH 5
#define INSTRUCTION_COUNT 8
void* instructions[INSTRUCTION_COUNT] = {&mov, &movc, &jmp, &jc, &cmp, &add, &out, &in};