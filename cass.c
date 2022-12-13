#include "./instructions.c"

/**
 *@brief Initializes file contents into a memory buffer
 *
 * @param file
 * @return struct FileContents
 */
struct FileContents fileToArr(FILE* file) {
	// Get length of file in lines
	int fileLenLines = getFileLengthLines(file);
	int fileLen = getFileLength(file);

	// Initialize buffer
	struct FileContents contents;
	contents.length = 0;

	// Allocate enough memory for the buffer
	contents.data = malloc(sizeof(char*) * fileLenLines);
	char* lines = malloc(sizeof(char) * fileLen + 1);
	// Place 'E' at the end of the buffer
	lines[fileLen] = 'E';

	// line pointer at index 0 of lines
	char* line = lines;
	// Read file into buffer
	char c;
	for (int i = 0; i < fileLen; i++) {
		c = fgetc(file);
		if (c == '\n') {
			contents.data[contents.length] = line;
			contents.length++;
			line = &lines[i + 1];
			continue;
		}
		lines[i] = c;
	}

	return contents;
}

/**
 * @brief Find the start of .CODE and .DATA sections and save that info to code and data variables
 * @param contents
 */
void getSections(struct FileContents contents) {
	dataContents.length = codeContents.length = -1;
	// Find the first line that whit .data
	int i = 0;
	while (i < contents.length) {
		char* contentDataI = contents.data[i];
		if (strstr(contents.data[i], ".code") != NULL) {
			dataContents.length = i;
			codeContents.length = contents.length - i - 1;
			break;
		}
		i++;
	}

	dataContents.data = &contents.data[1];
	dataContents.length--;

	// Set code.data to the contents.data text.length + 1 index
	codeContents.data = &contents.data[dataContents.length + 2];

	if (dataContents.length == 0) printException("No data in .data section", WARNING, -1);
	if (codeContents.length == -1) exitMsg(RED "Error: No .code section" RESET, 1);
	if (codeContents.length == 0) printException("No instructions in .code section", WARNING, -1);
}

void processData() {
	for (int i = 0; i < dataContents.length; i++) {
		// If the line is empty, skip it
		if (dataContents.data[i][0] == '\0') continue;

		// Get the line and its length
		char* line = dataContents.data[i];
		int strLen = strlen(line);

		// Get the first 3 chars of the line (the data type)
		char* dataType = malloc(sizeof(char) * 4);
		strncpy(dataType, line, 3);
		dataType[3] = '\0';

		// Get the data name
		char* dataName = malloc(sizeof(char) * strLen);
		sscanf(line, "%*s %s", dataName);
		printf("Data name: %s\n", dataName);
	}
}

int getArgType(char* arg) {
	// Check if it's a register
	if (arg[0] == '$') {
		int reg = atoi(&arg[1]);
		if (reg >= 0 && reg < REGISTER_COUNT) return REGISTER;
	}

	// Check if it's a number
	if (isdigit(arg[0]) || arg[0] == '-') {
		int num = atoi(arg);
		if (num >= -MAX_NUMBER_SIZE && num <= MAX_NUMBER_SIZE) return NUMBER;
	}

	// Check if it's a label
	if (getLabel(arg).lineNum != -1) return LABEL;

	// Check if it's a data pointer
	if (arg[0] == '#') return DATA_POINTER;

	return UNKNOWN;
}

int processLabel(int lineNum) {
	// Get the label name
	char* str = codeContents.data[lineNum];
	char* labelName = malloc(sizeof(char) * strlen(str));
	strcpy(labelName, str);
	int strLen = strlen(labelName);
	labelName[strLen - 1] = '\0';

	// Check if the label already exists
	struct Label duplicateLabel = getLabel(labelName);
	int labelExists = duplicateLabel.lineNum;

	if (labelExists != -1) {
		printException(" Label '" YELLOW "%s" RESET "' is already defined", WARNING, lineNum);
		printLine(labelExists);
		printf("\n");
		return 0;
	}
	// Should I return 0 or one depending if a function succeeds or not
	// Allocate memory for the label
	labels.data = realloc(labels.data, sizeof(struct Label) * (labels.length + 1));

	// Set the label's name
	labels.data[labels.length].name = labelName;

	// Set the label's line number
	labels.data[labels.length].lineNum = lineNum;

	// program.length - 1 because when we jump to the label, PC still gets incremented in the for loop in main.
	labels.data[labels.length].PC = program.length - 1;
	labels.length++;
	return 0;
}

int convertArg(char* arg, int argType) {
	int val = 0;
	switch (argType) {
		{
		case REGISTER:
			// Register
			sscanf(arg, "$%d", &val);
			break;
		case NUMBER:
			// Number
			return atoi(arg);
		case LABEL:
			// Label
			return getLabel(arg).PC;
		// case DATA_POINTER:
		// 	// Data pointer
		// 	return getData(arg).lineNum;
		default:
			printException(" Invalid argument type", ERROR, -1);
		}
	}
	return val;
}

int processInstruction(int lineNum) {
	char* line = codeContents.data[lineNum];
	int strLen = strlen(line);

	// Get the instruction name and lowercase it
	char* instruction = malloc(sizeof(char) * (strLen + 1));
	strcpy(instruction, line);
	instruction = strtok(instruction, " ");
	toLowerStr(instruction);

	// Allocate memory for the arguments
	char* arg = malloc(sizeof(char) * strLen);
	int* args = NULL;
	int* argTypes = NULL;
	int argCount = 0;
	// Get the arguments and keep instruction.
	// Remove any \r or spaces or ',' from the arguments
	// strtok(NULL, " ,\r\t\v\f\n\0")
	char c = '\0';
	int argLen = 0;

	for (int i = strlen(instruction) + 1; i < strLen + 1; i++) {
		c = line[i];
		if (c == ' ' || c == ',' || c == '\r' || c == '\t' || c == '\v' || c == '\f' || c == '\n' || c == '\0') {
			if (argLen == 0) continue;
			arg[argLen] = '\0';
			argLen = 0;
			argCount++;
			int argType = getArgType(arg);
			args = realloc(args, sizeof(int) * argCount);
			argTypes = realloc(argTypes, sizeof(int) * argCount);
			args[argCount - 1] = convertArg(arg, argType);
			argTypes[argCount - 1] = argType;
			continue;
		}
		arg[argLen++] = c;
	}
	free(arg);
	int foundInstruction = 0;
	int errorDetected = 0;
	// We loop until we find the instruction
	for (int instructionIndex = 0; instructionIndex < INSTRUCTION_COUNT; instructionIndex++) {
		struct Instruction* inst = (struct Instruction*)instructions[instructionIndex];
		// Skip if the instruction name doesn't match
		if (strcmp(instruction, inst->name) != 0) continue;
		// Check if the instruction is valid
		if (argCount != inst->argCount) {
			// "Invalid number of arguments for instruction " YELLOW "mov\n" RESET "Expected " YELLOW "%d" RESET
			// " argument/s, got " YELLOW "%d" RESET "\n"
			printException("Invalid number of arguments", ERROR, lineNum);

			// print '^' under the arguments
			int instructionLen = strlen(inst->name) + 1;
			int argLen = strlen(line) - instructionLen;
			printf("\t    ");
			for (int i = 0; i < instructionLen; i++) printf(" ");
			for (int i = 0; i < argLen; i++) printf("^");

			printf(" - Expected " YELLOW "%d" RESET " argument/s, got " YELLOW "%d" RESET "\n\n", inst->argCount,
				   argCount);
			errorDetected = 1;
		}
		// Check if the arguments are valid
		for (int j = 0; j < argCount; j++) {
			if (argTypes[j] == inst->argTypes[j]) continue;
			printException("Invalid argument type", ERROR, lineNum);
			// Get to the beginning of the arguments

			printf("\tExpected " YELLOW "%s" RESET ", got " YELLOW "%s" RESET " for argument " YELLOW "%d" RESET "\n\n",
				   argTypeMap[inst->argTypes[j]], argTypeMap[argTypes[j]], j + 1);
			errorDetected = 1;
		}
		foundInstruction = 1;
		if (errorDetected) break;

		// Add to program
		program.instructions = realloc(program.instructions, sizeof(struct ProgramInstruction) * (program.length + 1));
		struct ProgramInstruction* progInst = malloc(sizeof(struct ProgramInstruction));
		progInst->lineNum = lineNum;
		progInst->args = args;
		progInst->instructionIndex = instructionIndex;

		program.instructions[program.length] = *progInst;
		program.length++;
		// progInst = {lineNum, convertedArgs, instructionIndex};
		break;
	}
	// If the instruction wasn't found, print a warning
	if (!foundInstruction) {
		printf(YELLOW "Warning:" RESET " Unknown instruction '" YELLOW "%s" RESET "' at " MAGENTA "line %d" RESET
					  " - Ignoring\n",
			   instruction, getGlobalLineNum(lineNum));
		printLine(lineNum);
		printf("\n");
	}
	free(instruction);
	return errorDetected;
}

/**
 *@brief Check if there are any errors in the code
 */
void processProgram() {
	int success = 1;
	// Check if there are any labels that are not defined
	for (int i = 0; i < codeContents.length; i++) {
		char* line = codeContents.data[i];
		if (line[0] == '\n' || line[0] == '\0') continue;

		// If the line is a comment, skip it
		if (isComment(line)) continue;

		// If the line is a label, check if it already exists, if not, add it to the labels array
		if (isLabel(line)) {
			if (processLabel(i) != 0) success = 0;
			continue;
		}

		// Otherwise the line is an instruction
		if (processInstruction(i) != 0) success = 0;
	}
	if (!success) {
		printException("Errors were detected in the code and the program will not run", ERROR, -1);
		exit(1);
	}
}

void printDebug(int PC) {
	clear();
	printf("   Line: %d | Compare Flag: %d\n", getGlobalLineNum(PC), compareFlag);

	for (int i = 0; i < codeContents.length; i++) {
		if (i < 0 || i >= codeContents.length) continue;

		// Line without \n at the end
		char* line = malloc(sizeof(char) * strlen(codeContents.data[i]));
		strcpy(line, codeContents.data[i]);
		line = trimStr(line);
		line[strlen(line)] = '\0';

		// If we are at the PC, print a '>' before the line
		if (i == PC)
			printf(YELLOW ">" MAGENTA " %3d" RESET " | " YELLOW "%s" RESET, getGlobalLineNum(i), line);
		else
			printf(MAGENTA "  %3d " RESET "| %s", getGlobalLineNum(i), line);

		// Print empty spaces after the line
		for (int j = 0; j < 20 - strlen(line); j++) printf(" ");

		// If i is smaller then register count, print the register
		// Print 3 registers per line
		if (i * 3 < REGISTER_COUNT) {
			printf("| ");
			for (int j = 0; j < 3; j++) {
				if (i * 3 + j >= REGISTER_COUNT) break;
				printf("%2d: %5d | ", i * 3 + j, registers[i * 3 + j]);
			}
		}
		printf("\n");
	}
}

// Get arguments from main
int main(int argc, char* argv[]) {
	// Check if --debug is passed
	argc = 2;
	argv[1] = "./tests/test.asm";

	int debug = 0;
	if (argc > 2 && strcmp(argv[2], "--debug") == 0) debug = 1;

	// The first argument is the filename
	if (argc < 2) {
		printf("Usage: cass <filename>");
		return 1;
	}
	char* filename = argv[1];
	// Check if file exists
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		printf("File '%s' does not exist", filename);
		return 1;
	}

	// Read file
	struct FileContents contents = fileToArr(file);
	// Find the sections
	getSections(contents);
	processData();
	processProgram();
	// Index labels
	// indexLabels();

	int lastOutputLine = 0;
	for (int PC = 0; PC < program.length; PC++) {
		struct ProgramInstruction programInstruction = program.instructions[PC];
		if (debug) {
			printDebug(programInstruction.lineNum);
			printf("Output\n");
			for (int i = 0; i < outputLines; i++) printf("%s", output[i]);
			printf("\n");

		} else if (lastOutputLine != outputLines) {
			printf("%s", output[outputLines - 1]);
			lastOutputLine = outputLines;
		}

		// Interpret the instruction
		((struct Instruction*)instructions[programInstruction.instructionIndex])->func(&PC, programInstruction.args);
		usleep(200000);
	}

	return 0;
}