#include "headers.h"
#include "globals.c"
#include "_utils.c"
#include "instructions.c"
#include "help.c"
#include "process.c"
#include "keyboard.c"
#include "debug.c"

// TODO: Implement a .config section that can be used to set options without having to use command line arguments
// TODO: Improve the debug mode by adding a way to step through the program line by line, a way to set breakpoints,
// modify registers, etc. (Terminal UI)

/**
 * @brief Validate the registers
 */
void checkAndFixRegisters() {
	int isValid = 1;
	// If $0 has a different value than 0, set it to 0
	if (registers[0] != 0) {
		registers[0] = 0;
		isValid = 0;
		if (verbose == 0) return;
		printException(YELLOW "Warning:" RESET " Register " MAGENTA "$0" RESET " has the value of " MAGENTA "%d" RESET
							  " - Setting it to 0\n",
					   WARNING, -1, registers[0]);
	}

	// if the value in the register is bigger than the register size, wrap it
	for (int i = 0; i < registerCount; i++) {
		if (registers[i] <= registerSize) continue;
		int oldVal = registers[i];
		isValid = 0;
		// Wrap the value
		registers[i] = registers[i] % registerSize;
		if (verbose == 0) continue;
		printException("Register " MAGENTA "$%d" RESET " with the value of " MAGENTA "%d" RESET
					   " is bigger than the register size of " MAGENTA "%d" RESET " - Wrapping\n",
					   WARNING, -1, i, oldVal, registerSize);
	}
	if (strict == 1 && isValid == 0)
		exitMsg("Exiting due to" RED " strict mode" RESET "\nUse -v to see runtime warnings\n", 1);
}

int lastOutputLine = 0;
void runInstruction(int* PC) {
	if (debug) {
		printDebug(*PC);
	} else if (lastOutputLine != output.length) {
		printf("%s", output.data[output.length - 1]);
		lastOutputLine = output.length;
	}
	// Interpret the instruction
	struct ProgramInstruction programInstruction = program.instructions[*PC];
	((struct Instruction*)instructions[programInstruction.instructionIndex])->func(PC, programInstruction.args);
	checkAndFixRegisters();
}

int main(int argc, char* argv[]) {
	if (argc == 1) printHelp();
	if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) printHelp();
	if (argc == 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0)) printVersion();
	if (argc > 2) validateOptions(argc, argv);

	// Initialize the registers
	registers = calloc(registerCount, sizeof(int));
	maxNumberSize = (int)pow(2, registerSize * 8 - 1) - 1;

	// Open the file and check if it exists
	char* filename = argv[1];
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		printf("File '%s' does not exist", filename);
		return 1;
	}

	// Read file
	contents = fileToArr(file);
	fclose(file);

	// Find the section indexes for .data and .code
	for (int i = 0; i < contents.length; i++) {
		if (strstr(contents.data[i], ".data") != NULL) dataSection.start = i;
		if (strstr(contents.data[i], ".code") != NULL) {
			dataSection.end = i;
			dataSection.length = dataSection.end - dataSection.start;

			codeSection.start = i;
			codeSection.end = contents.length;
			codeSection.length = codeSection.end - codeSection.start;
			break;
		}
	}

	// Validate the data and code sections
	if (dataSection.start == -1)
		printException("No .data section", WARNING, -1, NULL);
	else if (dataSection.length == 1)
		printException("No data in .data section", WARNING, -1, NULL);
	if (codeSection.length == -1) exitMsg(RED "Error: No .code section" RESET, 1);
	if (codeSection.length == 0) printException("No instructions in .code section", WARNING, -1, NULL);

	// Process the data and program sections
	// if either of them return 1, error detected
	int dataExists = (dataSection.start != -1 && dataSection.length != 1);
	if ((dataExists && processData(contents, dataSection)) || processProgram(contents, codeSection)) {
		printException("Errors were detected in the code and the program will not run", ERROR, -1, NULL);
		exit(1);
	}

	kInit();
	// Run the program
	int interval = 1000000 / speed;

	int PC = 0;
	// Use clock() to get the time difference
	int start = clock();
	int end = 0;
	int time = 0;

	int c = 0;

	int runOne = 0;

	struct winsize w;

	while (1) {
		end = clock();
		time = end - start;

		// Get screen size
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		screenWidth = w.ws_col;
		screenHeight = w.ws_row;

		// Check if a key was pressed
		if ((c = getKeyPress()) != -1) {
			printf("Key pressed: %d\n", c);
			switch (c) {
			case 267:
			case 266:
				if (paused) runOne = 1;
				break;
			case '+':
			case '=':
				if (speed < 100) speed++;
				interval = 1000000 / speed;
				break;
			case '-':
			case '_':
				if (speed > 1) speed--;
				interval = 1000000 / speed;
				break;
			case 'p':
			case 217:
				paused = !paused;
				if (debug) printDebug(PC);
				break;
			case 27:
				return 0;
			default:
				break;
			}
		}

		// If the time difference is bigger than the interval, run the instruction
		if ((time > interval && !paused) || runOne) {
			runInstruction(&PC);
			PC++;
			if (PC >= program.length) break;
			start = clock();
			runOne = 0;
		}
	}

	// Free the memory
	free(registers);
	// freeArr(&contents);

	return 0;
}