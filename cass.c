#include "headers.h"
#include "globals.c"
#include "_utils.c"
#include "instructions.c"

// TODO: Implement a .config section that can be used to set options without having to use command line arguments
// TODO: Send all the program processing function to a new file
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
	contents.data = calloc(fileLenLines, sizeof(char*));
	char* lines = calloc(fileLen + 1, sizeof(char));
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
 * @brief Processes a string type data value in the data section
 *
 * @param line
 * @param lineNum
 * @param value
 * @return int - 1 if error, 0 if no error
 */
int processDataString(char* line, int lineNum, void** value) {
	int isValid = 1;

	// Make sure there are two quotes "
	int strLen = strlen(line);

	// Only get the data value which is the third "word"
	char* dataValue = calloc(strLen, sizeof(char));
	sscanf(line, "%*s %*s %[^\n\r]", dataValue);

	// Quote checking
	int quoteCount = 0;
	int firstQuote = -1;
	int lastQuote = -1;

	// Count the number of quotes
	for (int i = 0; i < strLen; i++) {
		if (dataValue[i] == '"' && (i == 0 || dataValue[i - 1] != '\\')) {
			quoteCount++;
			if (firstQuote == -1) {
				firstQuote = i;
				continue;
			}
			lastQuote = i;
		};
	}

	// If there are no quotes, or there are more than 2 quotes, throw an error
	if (quoteCount < 2) {
		printException("Invalid string", ERROR, lineNum);
		printf("String must be surrounded by quotes (\").\n");
		free(dataValue);
		return 1;
	}
	if (quoteCount > 2) {
		printException("Invalid string", ERROR, lineNum);
		printf("String must only contain one set of quotes (" YELLOW "\"STRING\"" RESET ").\n");
		printf("Make sure you escape other quotes with a backslash (" YELLOW "\\\"" RESET ").\n");
		free(dataValue);
		return 1;
	}

	// Check if there is nothing else after the last quote
	if (quoteCount == 2 && line[strLen - 1] != '"')
		printException("Found extra characters after string (Ignoring)", WARNING, lineNum);

	// Remove the quotes by moving the string to the left without source and destination overlap
	memmove(dataValue, dataValue + 1, lastQuote - 1);
	dataValue[lastQuote - 1] = '\0';
	dataValue = realloc(dataValue, sizeof(char) * (strlen(dataValue) + 1));
	*value = dataValue;

	return !isValid;
}
/**
 * @brief Process the data section of the file line by line
 *
 * @param contents
 * @param dataSection
 * @return int - 1 if error, 0 if no error
 */
int processData(struct FileContents contents, struct SectionIndex dataSection) {
	int success = 1;
	dataList.data = malloc(sizeof(struct Data) * dataSection.length);
	for (int i = dataSection.start + 1; i < dataSection.end; i++) {
		int isValid = 1;

		// Get the line
		char* line = contents.data[i];

		// If the line is empty, skip it
		if (line[0] == '\0') continue;

		// Get the lines its length
		int strLen = strlen(line);

		// Get the first 3 chars of the line (the data type)
		char* dataTypeName = malloc(sizeof(char) * 4);
		strncpy(dataTypeName, line, 3);
		dataTypeName[3] = '\0';

		// Check if the data type is valid
		if (strcmp(dataTypeName, "int") != 0 && strcmp(dataTypeName, "str") != 0) {
			isValid = 0;
			printException("Invalid data type", ERROR, i);
		}
		int dataType = strcmp(dataTypeName, "int") == 0 ? TYPE_INT : TYPE_STR;
		free(dataTypeName);
		// Get the data name
		char* dataName = malloc(sizeof(char) * strLen);
		sscanf(line, "%*s %s", dataName);
		// Reallocate the data name to the correct size
		dataName = realloc(dataName, sizeof(char) * (strlen(dataName) + 1));

		// Check if the data name is already defined
		struct Data duplicateData = getData(dataList, dataName);
		if (duplicateData.lineNum != -1) {
			isValid = 0;
			printException("Variable name already defined", ERROR, i);
			printLine(duplicateData.lineNum);
			printf("\n");
		}

		// Check if the data name is a valid variable name
		// Make sure it starts with a letter, and only contains letters, numbers, '_' and '-'
		int isValidName = 1;
		if (!isalpha(dataName[0])) isValidName = 0;
		if (isValidName)
			for (int j = 1; j < strlen(dataName); j++) {
				if (isalnum(dataName[j]) || dataName[j] == '_' || dataName[j] == '-') continue;
				isValidName = 0;
				break;
			}
		if (!isValidName) {
			isValid = 0;
			printException("Invalid variable name", ERROR, i);
			printf("Variable name must start with a letter, and only contain letters, numbers, '_' or '-'.\n");
		}

		// Data value pointer is a pointer to the data value.
		void* dataValuePointer = NULL;
		// Get the data value
		char* dataValue = malloc(sizeof(char) * strLen);
		sscanf(line, "%*s %*s %[^\n\r]", dataValue);
		// Check if dataValue is empty
		if (dataValue[0] == '\0') {
			isValid = 0;
			printException("No value specified", ERROR, i);
		} else if (dataType == TYPE_INT) {
			// Get the data value again, but as an int
			int dataValueInt;
			sscanf(line, "%*s %*s %d", &dataValueInt);
			dataValuePointer = malloc(sizeof(int));
			*(int*)dataValuePointer = dataValueInt;
		} else if (processDataString(line, i, &dataValuePointer))
			isValid = 0;

		free(dataValue);
		// If the variable is not valid, skip it
		if (!isValid) {
			success = 0;
			continue;
		}

		// Add the data to the data array
		struct Data data;
		data.lineNum = i;
		data.name = dataName;
		data.type = dataType;
		data.value = dataValuePointer;

		// Add the data to the data list
		dataList.data[dataList.length] = data;
		dataList.length++;
	}
	// Reallocate the data list to the correct size
	dataList.data = realloc(dataList.data, sizeof(struct Data) * dataList.length);

	return !success;
}

/**
 * @brief Process the label at the given line
 *
 * @param lineNum
 * @return int - 1 if error, 0 if no error
 */
int processLabel(int lineNum) {
	// Get the label name
	char* str = contents.data[lineNum];
	char* labelName = calloc(strlen(str) + 1, sizeof(char));
	strcpy(labelName, str);
	int strLen = strlen(labelName);
	labelName[strLen - 1] = '\0';

	// Check if the label already exists
	struct Label duplicateLabel = getLabel(labels, labelName);
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

int processInstruction(int lineNum) {
	int errorDetected = 0;
	char* line = contents.data[lineNum];
	int strLen = strlen(line);

	// Get the instruction name and lowercase it
	char* instruction = malloc(sizeof(char) * (strLen + 1));
	strcpy(instruction, line);
	instruction = strtok(instruction, " ");
	toLowerStr(instruction);

	// Allocate memory for the arguments
	char* arg = malloc(sizeof(char) * (strLen + 1));
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
		if (!(c == ' ' || c == ',' || c == '\r' || c == '\t' || c == '\v' || c == '\f' || c == '\n' || c == '\0')) {
			arg[argLen++] = c;
			continue;
		}
		if (argLen == 0) continue;
		arg[argLen] = '\0';
		argLen = 0;
		argCount++;

		int argType = getArgType(dataList, arg);

		if (validateArgument(dataList, arg, argType, lineNum) == 1) {
			errorDetected = 1;
			continue;
		}

		args = realloc(args, sizeof(int) * argCount);
		argTypes = realloc(argTypes, sizeof(int) * argCount);
		// Check if the argument is valid

		args[argCount - 1] = convertArg(dataList, arg, argType);
		argTypes[argCount - 1] = argType;
	}
	free(arg);

	if (errorDetected) {
		free(instruction);
		free(args);
		free(argTypes);
		return 1;
	}

	int foundInstruction = 0;
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
		struct ProgramInstruction progInst;
		progInst.lineNum = lineNum;
		progInst.args = args;
		progInst.instructionIndex = instructionIndex;

		// Assign the pointer to the program
		program.instructions[program.length] = progInst;
		program.length++;

		// Allocate some more memory for the program
		char* someMem = calloc(1000, sizeof(char));
		free(someMem);
		// progInst = {lineNum, convertedArgs, instructionIndex};
		break;
	}
	// If the instruction wasn't found, print a warning
	if (!foundInstruction) {
		printf(YELLOW "Warning:" RESET " Unknown instruction '" YELLOW "%s" RESET "' at " MAGENTA "line %d" RESET
					  " - Ignoring\n",
			   instruction, lineNum);
		printLine(lineNum);
		printf("\n");
	}
	free(argTypes);
	free(instruction);
	return errorDetected;
}

/**
 * @brief Check if there are any errors in the code
 * @return 0 if there are no errors, 1 if there are errors
 */
int processProgram(struct FileContents contents, struct SectionIndex codeSection) {
	int success = 1;
	// Check if there are any labels that are not defined
	for (int i = codeSection.start + 1; i < codeSection.end; i++) {
		char* line = contents.data[i];
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
	return !success;
}

/**
 * @brief Print the debug view of the program
 * @param PC The program counter
 */
void printDebug(int PC) {
	clear();
	printf("   Line: %d | Compare Flag: %d\n", PC, compareFlag);

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

// This could be a struct
// TODO: Make the help a struct with a name and a description and validation function pointer.
// A.K.A. Make it more complex but also better and easier to use
// C devs do be like: "Hmmmm. I can make this into a struct". And I think that's beautiful.
void printHelp() {
	printf("Usage: cass <filename> [arguments]\n");
	printf("Arguments:\n");
	printf("  --debug, -d\t\t\t\tPrint the debug view of the program\n");
	printf("  --registers <amount>, -r <amount>\tHow many registers the program has ($0 is not counted)\n");
	printf("  --register-size <amount>, -S <amount>\tHow much bytes a register can hold\n");
	printf("  --verbose, -v\t\t\t\tPrint all the normally ignored warnings and errors\n");
	printf("  --version, -V\t\t\t\tPrint the version of the program\n");
	printf("  --speed <value>, -s <value>\t\tHow many instructions to execute per second (max 100)\n");
	printf("  --strict\t\t\t\tExit with an error if there are any runtime warnings\n");
	printf("  --help, -h\t\t\t\tPrint this help message\n");
}

/**
 *@brief Validate the argument value
 *
 * @param argc - The argument count
 * @param argv - The argument array
 * @param index - The index of the argument
 * @param argName - The name of the argument
 */
void validateArgumentValue(int argc, char* argv[], int index, char* argName) {
	// if value begins with a '-', it is another argument
	if (index + 1 >= argc || argv[index + 1][0] == '-') {
		printf("Expected an argument for %s\n", argName);
		exit(1);
	}
	int count = atoi(argv[index + 1]);
	if (count < 1) {
		printf("Expected a positive integer for %s\n", argName);
		exit(1);
	}
}

/**
 * @brief Validate the registers
 */
void checkAndFixRegisters() {
	int isValid = 1;
	// If $0 has a different value than 0, set it to 0
	if (registers[0] != 0) {
		registers[0] = 0;
		if (verbose == 0) return;
		printf(YELLOW "Warning:" RESET " Register " MAGENTA "$0" RESET " has the value of " MAGENTA "%d" RESET
					  " - Setting it to 0\n",
			   registers[0]);
		isValid = 0;
	}

	// if the value in the register is bigger than the register size, wrap it
	for (int i = 0; i < registerCount; i++) {
		if (registers[i] <= registerSize) continue;
		int oldVal = registers[i];
		// Wrap the value
		registers[i] = registers[i] % registerSize;
		if (verbose == 0) continue;
		printf(YELLOW "Warning:" RESET " Register " MAGENTA "$%d" RESET " with the value of " MAGENTA "%d" RESET
					  " is bigger than the register size of " MAGENTA "%d" RESET " - Wrapping\n",
			   i, oldVal, registerSize);
		isValid = 1;
	}
	if (strict == 1 && isValid == 0) exitMsg("Exiting due to" RED " strict mode" RESET, 1);
}

int main(int argc, char* argv[]) {
	// The first argument is the filename
	if (argc < 2) {
		printf("Usage: cass <filename>");
		return 1;
	}

	// If help is the first argument, print the help
	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		printHelp();
		return 0;
	}

	// Other Arguments:
	int debug = 0;
	int speed = 1;

	// Parse the arguments
	// Im not gonna comment this, its pretty self explanatory, but also it will be changed soon
	for (int i = 2; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printHelp();
			return 0;
		}

		if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
			debug = 1;
			continue;
		}
		if (strcmp(argv[i], "--registers") == 0 || strcmp(argv[i], "-r") == 0) {
			validateArgumentValue(argc, argv, i, argv[i]);
			registerCount = atoi(argv[i + 1]);
			if (registerCount > MAX_REGISTER_COUNT) {
				printf("Register count cannot be larger than %d\n", MAX_REGISTER_COUNT);
				return 1;
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "--register-size") == 0 || strcmp(argv[i], "-s") == 0) {
			validateArgumentValue(argc, argv, i, argv[i]);
			registerSize = atoi(argv[i + 1]);
			if (registerSize > MAX_REGISTER_SIZE) {
				printf("Register size cannot be larger than %d\n", MAX_REGISTER_SIZE);
				return 1;
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
			verbose = 1;
			continue;
		}
		if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
			printf("Cass version %s\n", VERSION);
			return 0;
		}
		if (strcmp(argv[i], "--speed") == 0 || strcmp(argv[i], "-S") == 0) {
			validateArgumentValue(argc, argv, i, argv[i]);
			speed = atoi(argv[i + 1]);
			if (speed >= 100)
				printException("Speed larger or equal to 100 is considered as instant execution\n", INFO, -1);
			i++;
			continue;
		}
		if (strcmp(argv[i], "--strict") == 0) {
			strict = 1;
			continue;
		}
		printf("Unknown argument '%s'\n", argv[i]);
		return 1;
	}

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
		printException("No .data section", WARNING, -1);
	else if (dataSection.length == 1)
		printException("No data in .data section", WARNING, -1);
	if (codeSection.length == -1) exitMsg(RED "Error: No .code section" RESET, 1);
	if (codeSection.length == 0) printException("No instructions in .code section", WARNING, -1);

	// Process the data and program sections
	// if either of them return 1, error detected
	int dataExists = (dataSection.start != -1 && dataSection.length != 1);
	if ((dataExists && processData(contents, dataSection)) || processProgram(contents, codeSection)) {
		printException("Errors were detected in the code and the program will not run", ERROR, -1);
		exit(1);
	}

	// Run the program
	int lastOutputLine = 0;
	int delay = 1000000 / speed;
	for (int PC = 0; PC < program.length; PC++) {
		struct ProgramInstruction programInstruction = program.instructions[PC];
		if (debug) {
			printDebug(programInstruction.lineNum);
			printf("Output\n");
			for (int i = 0; i < output.length; i++) printf("%s", output.data[i]);
			printf("\n");
		} else if (lastOutputLine != output.length) {
			printf("%s", output.data[output.length - 1]);
			lastOutputLine = output.length;
		}
		// Interpret the instruction
		((struct Instruction*)instructions[programInstruction.instructionIndex])->func(&PC, programInstruction.args);
		checkAndFixRegisters();
		usleep(delay);
	}

	return 0;
}