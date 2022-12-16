
#include "headers.h"
#include "globals.c"
#include "_utils.c"

// ========================= INSTRUCTIONS =========================

// -----------------MOV-----------------
void movFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	registers[reg1] = registers[reg2];
}
struct Instruction mov = {"mov", 2, (int[]){REGISTER, REGISTER}, (void*)movFunc};

// -----------------MOVC-----------------
void movcFunc(int* PC, int args[]) {
	int reg = args[0];
	int value = args[1];
	registers[reg] = value;
}
struct Instruction movc = {"movc", 2, (int[]){REGISTER, NUMBER}, (void*)movcFunc};

// -----------------JMP-----------------
void jmpFunc(int* PC, int args[]) { *PC = args[0]; }
struct Instruction jmp = {"jmp", 1, (int[]){LABEL}, (void*)jmpFunc};

// -----------------JC-----------------
void jcFunc(int* PC, int args[]) {
	if (registers[0] == 0) *PC = args[0];
}
struct Instruction jc = {"jc", 1, (int[]){LABEL}, (void*)jcFunc};

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
struct Instruction cmp = {"cmp", 2, (int[]){REGISTER, REGISTER}, (void*)cmpFunc};

// -----------------ADD-----------------
void addFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	int sum = registers[reg1] + registers[reg2];
	// Check if the register overflows
	if (sum > MAX_NUMBER_SIZE || sum < -MAX_NUMBER_SIZE) {
		registers[reg1] = wrap(sum, MAX_NUMBER_SIZE, -MAX_NUMBER_SIZE);
		printf(YELLOW "Warning:" RESET " Register " YELLOW "$%d" RESET " overflowed at " MAGENTA "line %d" RESET "\n\n",
			   reg1, *PC);
		return;
	}
	registers[reg1] = sum;
}
struct Instruction add = {"add", 2, (int[]){REGISTER, REGISTER}, (void*)addFunc};

// -----------------OUT-----------------
void outFunc(int* PC, int args[]) {
	int reg = args[0];
	char* out = malloc(10 * sizeof(char));
	sprintf(out, "%d", registers[reg]);
	pushString(&output, out);
}
struct Instruction out = {"out", 1, (int[]){REGISTER}, (void*)outFunc};

// -----------------OUTS----------------
void outsFunc(int* PC, int args[]) {
	char* str = getDataStr(dataList, args[0]);
	pushString(&output, str);
}
struct Instruction outs = {"outs", 1, (int[]){DATA_POINTER_STR}, (void*)outsFunc};

// -----------------OUTN----------------
void outnFunc(int* PC, int args[]) { pushString(&output, "\n"); }
struct Instruction outn = {"outn", 0, (int[]){}, (void*)outnFunc};

// -----------------IN-----------------
void inFunc(int* PC, int args[]) {
	int reg = args[0];
	scanf("%d", &registers[reg]);
}
struct Instruction in = {"in", 1, (int[]){REGISTER}, (void*)inFunc};

// -----------------RAND-----------------
void randFunc(int* PC, int args[]) {
	srand(time(NULL));
	int reg = args[0];
	registers[reg] = rand() % 100;
}
struct Instruction randInst = {"rand", 1, (int[]){REGISTER}, (void*)randFunc};

// -----------------MOVED-----------------
void movedFunc(int* PC, int args[]) {
	// Move data int to register
	int reg = args[0];
	int value = getDataInt(dataList, args[1]);
	registers[reg] = value;
}
struct Instruction movedInst = {"moved", 2, (int[]){REGISTER, DATA_POINTER_INT}, (void*)movedFunc};

#define INSTRUCTION_COUNT 12
void* instructions[INSTRUCTION_COUNT] = {&mov, &movc, &jmp,	 &jc, &cmp,		 &add,
										 &out, &outs, &outn, &in, &randInst, &movedInst};