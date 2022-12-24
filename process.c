#include "headers.h"
#include "globals.c"
#include "_utils.c"
#include "instructions.c"

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