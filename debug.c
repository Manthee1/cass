#include "headers.h"
#include "globals.c"

/**
 * @brief Print the debug view of the program
 * @param PC The program counter
 */
void printDebug(int PC) {
	clear();

	struct ProgramInstruction programInstruction = program.instructions[PC];
	int lineNum = programInstruction.lineNum;
	int currentOutputIndex = output.length > screenHeight - 6 ? output.length - (screenHeight - 6) : 0;

	// Runnning or paused
	paused ? printf(BLACK BG_BLUE " ⏸︎ PAUSED  (F6) " RESET)
		   : printf(BLACK BG_YELLOW " ▶︎ RUNNING (F6) " RESET);

	printf(" | ");
	// Speed
	char* speedStr = itoa(speed);
	printf("Speed:" BOLD YELLOW " %s " RESET "(-/+)", speed >= MAX_SPEED ? "MAX" : speedStr);
	free(speedStr);

	printf(" | ");

	printf("\n");

	for (int i = 0; i < screenWidth; i++) printf("-");

	printf("       | Program |--------------  | Registers |------  <Output>\n");
	for (int i = 0; i < screenHeight - 6; i++) {
		// Print the code

		if (i < codeSection.length - 1) {
			int index = i + codeSection.start + 1;
			char* line = contents.data[index];
			// If we are at the PC, print a '>' before the line
			if (index == lineNum) {
				if (paused)
					printf(BLUE " >" MAGENTA " %3d" RESET " | " BLUE "%s " RESET, index, line);
				else
					printf(YELLOW " >" MAGENTA " %3d" RESET " | " YELLOW "%s " RESET, index, line);
			} else
				printf(MAGENTA "   %3d " RESET "| %s ", index, line);

			for (int j = 0; j < 20 - strlen(line); j++) printf(" ");
			printf(" | ");
			// Print empty spaces after the line
		} else if (i == codeSection.length - 1)
			printf("       ------------------------- ");
		else
			printf("                                 ");
		// Print the registers

		if (i < registerCount) {
			printf(" | ");
			printf(MAGENTA " $" RESET "%3d" RESET ": ", i);
			if (registers[i] == 0)
				printf(MAGENTA);
			else if (registers[i] < 0)
				printf(RED);
			else
				printf(GREEN);
			printf("%8d" RESET, registers[i]);
			printf(" | ");
		} else if (i == registerCount)
			printf(" ------------------- ");
		else
			printf("                     ");
		// Print the output
		if (output.length > 0 && output.length > i) printf("%s", output.data[i]);

		printf("\n");  // if (codeSection.end < i) break;
	}
	if (paused) {
		printf("Screen height: %d", screenHeight);
		printf("Use ⏵︎ or ⏷︎ to step through the program\n");
	}
}