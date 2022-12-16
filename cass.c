#include "./instructions.c"

// TODO: Improve error handling (correct data int, variable exists, etc)
// TODO: change codeContents and dataContents to be a line index instead of a pointer to the line
// TODO: Add runtime error handling (check if registers are valid, etc)
// TODO: Maybe split up the file into multiple files
// TODO: Add more comments and clean up code

struct FileContents contents = {0, NULL};

struct SectionIndex dataSection = {-1, -1, -1};
struct SectionIndex codeSection = {-1, -1, -1};

struct Labels labels = {0, NULL};
struct DataList dataList = {0, NULL};
struct Program program = {0, NULL};

void printLine(int lineNum) { printf(MAGENTA "\t%d " RESET "| %s\n", lineNum, contents.data[lineNum]); }
// ------- ERROR HANDLING -------
enum EXCEPTION_TYPE { ERROR, WARNING, INFO };

void printException(char* message, enum EXCEPTION_TYPE type, int lineNum) {
	switch (type) {
	case ERROR:
		printf(RED "Error" RESET);
		break;
	case WARNING:
		printf(YELLOW "Warning" RESET);
		break;
	case INFO:
		printf(GREEN "Info" RESET);
		break;
	}
	if (lineNum == -1) {
		printf(": %s\n", message);
		return;
	}
	printf(" on line " MAGENTA "%d" RESET ": %s\n", lineNum, message);
	printLine(lineNum);
}

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

int findCodeSection(struct FileContents contents) {
	// Find the first line that has .code
	for (int i = 0; i < contents.length; i++)
		if (strstr(contents.data[i], ".code") != NULL) return i;
	return -1;
}

int processDataString(char* line, int lineNum, void** value) {
	int isValid = 1;
	// Make sure there are two quotes "
	int strLen = strlen(line);

	char* dataValue = malloc(sizeof(char) * strLen);
	sscanf(line, "%*s %*s %[^\n\r]", dataValue);

	int quoteCount = 0;
	int firstQuote = -1;
	int lastQuote = -1;
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

	// Remove the quotes with strncpy
	strncpy(dataValue, dataValue + 1, lastQuote - 1);
	dataValue[lastQuote - 1] = '\0';
	dataValue = realloc(dataValue, sizeof(char) * (strlen(dataValue) + 1));
	*value = dataValue;

	return !isValid;
}

int processData(struct FileContents contents, struct SectionIndex dataSection) {
	int success = 1;
	dataList.data = malloc(sizeof(struct Data) * dataSection.length);
	for (int i = 0; i < dataSection.length; i++) {
		int isValid = 1;

		char* line = contents.data[i + dataSection.start];

		// If the line is empty, skip it
		if (line[0] == '\0') continue;

		// Get the line and its length
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

		void* dataValuePointer = NULL;
		// Check if dataValue will be empty
		char* dataValue = malloc(sizeof(char) * strLen);
		sscanf(line, "%*s %*s %[^\n\r]", dataValue);
		if (dataValue[0] == '\0') {
			isValid = 0;
			printException("No value specified", ERROR, i);
		} else if (dataType == TYPE_INT) {
			// Get the data value
			int dataValueInt;
			sscanf(line, "%*s %*s %d", &dataValueInt);
			dataValuePointer = malloc(sizeof(int));
			*(int*)dataValuePointer = dataValueInt;
		} else if (processDataString(line, i, &dataValuePointer))
			isValid = 0;

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
	if (getLabel(labels, arg).lineNum != -1) return LABEL;

	// Check if it's a data pointer
	if (arg[0] == '#') {
		char* dataName = malloc(sizeof(char) * strlen(arg));
		strcpy(dataName, &arg[1]);
		struct Data data = getData(dataList, dataName);
		free(dataName);
		if (data.lineNum != -1) return data.type == TYPE_INT ? DATA_POINTER_INT : DATA_POINTER_STR;
	}

	return UNKNOWN;
}

int processLabel(int lineNum) {
	// Get the label name
	char* str = contents.data[lineNum];
	char* labelName = malloc(sizeof(char) * strlen(str));
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

int convertArg(char* arg, int argType) {
	int val = 0;
	switch (argType) {
		{
		case REGISTER:
			sscanf(arg, "$%d", &val);
			break;
		case NUMBER:
			return atoi(arg);
		case LABEL:
			return getLabel(labels, arg).PC;
		case DATA_POINTER_INT:
		case DATA_POINTER_STR:
			val = 0;  // This is a hack to make the IDE happy
			char* dataName = malloc(sizeof(char) * strlen(arg));
			strcpy(dataName, &arg[1]);
			val = getDataIndex(dataList, dataName);
			free(dataName);
			break;
		default:
			printException("Invalid argument type", ERROR, -1);
		}
	}
	return val;
}

int processInstruction(int lineNum) {
	char* line = contents.data[lineNum];
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
			   instruction, lineNum);
		printLine(lineNum);
		printf("\n");
	}
	free(instruction);
	return errorDetected;
}

/**
 *@brief Check if there are any errors in the code
 *@return 0 if there are no errors, 1 if there are errors
 */
int processProgram(struct FileContents contents, struct SectionIndex codeSection) {
	int success = 1;
	// Check if there are any labels that are not defined
	for (int i = codeSection.start; i < codeSection.end; i++) {
		char* line = contents.data[codeSection.start + i];
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

void printDebug(int PC) {
	clear();
	printf("   Line: %d | Compare Flag: %d\n", PC, compareFlag);

	for (int i = codeSection.start; i < codeSection.end; i++) {
		// if (i < codeSection.start || i >= codeSection.end) continue;

		// Line without \n at the end
		char* line = malloc(sizeof(char) * strlen(contents.data[i]));
		strcpy(line, contents.data[i]);
		line = trimStr(line);
		line[strlen(line)] = '\0';

		// If we are at the PC, print a '>' before the line
		if (i - codeSection.start == PC)
			printf(YELLOW ">" MAGENTA " %3d" RESET " | " YELLOW "%s" RESET, i, line);
		else
			printf(MAGENTA "  %3d " RESET "| %s", i, line);

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
	contents = fileToArr(file);
	fclose(file);

	// Find the section indexes
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

	printf("Data Section: %d - %d\n", dataSection.start, dataSection.end);
	printf("Code Section: %d - %d\n", codeSection.start, codeSection.end);

	if (dataSection.start == -1)
		printException("No .data section", WARNING, -1);
	else if (dataSection.length == 1)
		printException("No data in .data section", WARNING, -1);
	int dataExists = (dataSection.start != -1 && dataSection.length != 1);

	if (codeSection.length == -1) exitMsg(RED "Error: No .code section" RESET, 1);
	if (codeSection.length == 0) printException("No instructions in .code section", WARNING, -1);

	// Process the data and program sections
	// if either of them return 1, error detected
	if ((dataExists && processData(contents, codeSection)) || processProgram(contents, codeSection)) {
		printException("Errors were detected in the code and the program will not run", ERROR, -1);
		exit(1);
	}

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