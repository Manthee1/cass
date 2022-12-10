
#include "headers.h"

// ========================= INSTRUCTIONS =========================

// -----------------MOV-----------------
void movFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	registers[reg1] = registers[reg2];
}
struct Instruction mov = {"mov", 2, (int[]){0, 0}, (void*)movFunc};

// -----------------MOVC-----------------
void movcFunc(int* PC, int args[]) {
	int reg = args[0];
	int value = args[1];
	registers[reg] = value;
}
struct Instruction movc = {"movc", 2, (int[]){0, 1}, (void*)movcFunc};

// -----------------JMP-----------------
void jmpFunc(int* PC, int args[]) { *PC = args[0]; }
struct Instruction jmp = {"jmp", 1, (int[]){2}, (void*)jmpFunc};

// -----------------JC-----------------
void jcFunc(int* PC, int args[]) {
	if (registers[0] == 0) *PC = args[0];
}
struct Instruction jc = {"jc", 1, (int[]){2}, (void*)jcFunc};

// -----------------CMP-----------------
void cmpFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	if (registers[reg1] == registers[reg2])
		registers[0] = 0;
	else if (registers[reg1] > registers[reg2])
		registers[0] = 1;
	else
		registers[0] = -1;
}
struct Instruction cmp = {"cmp", 2, (int[]){0, 0}, (void*)cmpFunc};

// -----------------ADD-----------------
void addFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
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
void outFunc(int* PC, int args[]) {
	int reg = args[0];
	char out[10];
	sprintf(out, "%d", registers[reg]);
	addToOutput(out);
}
struct Instruction out = {"out", 1, (int[]){0}, (void*)outFunc};

// -----------------OUTN----------------
void outnFunc(int* PC, int args[]) { addToOutput("\n"); }
struct Instruction outn = {"outn", 0, (int[]){}, (void*)outnFunc};

// -----------------IN-----------------
void inFunc(int* PC, int args[]) {
	int reg = args[0];
	scanf("%d", &registers[reg]);
}
struct Instruction in = {"in", 1, (int[]){0}, (void*)inFunc};

// -----------------RAND-----------------
void randFunc(int* PC, int args[]) {
	srand(time(NULL));
	int reg = args[0];
	registers[reg] = rand() % 100;
}
struct Instruction randInst = {"rand", 1, (int[]){0}, (void*)randFunc};

#define MAX_INSTRUCTION_LENGTH 5
#define INSTRUCTION_COUNT 10
void* instructions[INSTRUCTION_COUNT] = {&mov, &movc, &jmp, &jc, &cmp, &add, &out, &outn, &in, &randInst};