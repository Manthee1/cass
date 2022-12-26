#include "headers.h"
#include "globals.c"

/**
 * @brief Print the debug view of the program
 * @param PC The program counter
 */
void printDebug(int PC) {
	clear();

	struct ProgramInstruction programInstruction = program.instructions[PC];
	if (paused) {
		// Color the background of the current line
		printf("\033[48;5;238m");
		printf(LYELLOW " ⏸︎ PAUSED " RESET);
	} else
		printf(LYELLOW " ▶︎ RUNNING" RESET);
	printf("   Line: %d | Compare Flag: %d\n", PC, compareFlag);
	for (int i = 0; i < screenWidth; i++) printf("-");

	for (int i = codeSection.start + 1; i < codeSection.end; i++) {
		// if (i < codeSection.start || i >= codeSection.end) continue;

		// Line without \n at the end
		char* line = malloc(sizeof(char) * (strlen(contents.data[i]) + 1));
		strcpy(line, contents.data[i]);
		line = trimStr(line);
		line[strlen(line)] = '\0';

		// If we are at the PC, print a '>' before the line
		if (i == PC)
			printf(YELLOW ">" MAGENTA " %3d" RESET " | " YELLOW "%s" RESET, i, line);
		else
			printf(MAGENTA "  %3d " RESET "| %s", i, line);

		// Print empty spaces after the line
		for (int j = 0; j < 20 - strlen(line); j++) printf(" ");

		int index = i - codeSection.start - 1;
		// If i is smaller then register count, print the register
		// Print 3 registers per line
		if (index * 3 < registerCount) {
			printf("| ");
			for (int j = 0; j < 3; j++) {
				if (index * 3 + j >= registerCount) break;
				printf("%2d: %5d | ", index * 3 + j, registers[index * 3 + j]);
			}
		}
		printf("\n");
	}
}