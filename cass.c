#include "./instructions.c"

struct FileContents fileToArr(FILE* file) {
	// Get length of file
	int fileLen = 0;
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	rewind(file);
	struct FileContents contents;
	contents.length = 0;
	contents.data = malloc(sizeof(char*) * fileLen);
	char* line = NULL;
	size_t len = 0;
	size_t read;

	while ((read = getline(&line, &len, file)) != -1) {
		contents.length++;
		contents.data[contents.length - 1] = malloc(sizeof(char) * read);
		strcpy(contents.data[contents.length - 1], line);
	}
	free(line);
	return contents;
}

/**
 * @brief FInd the start of .TEXT and .DATA sections and save that info to data and text variables
 * @param contents
 */
void getSections(struct FileContents contents) {
	text.length = data.length = -1;
	// Find the first line that whit .data
	int i = 0;
	while (i < contents.length) {
		if (strstr(contents.data[i], ".data") != NULL) {
			text.length = i;
			data.length = contents.length - i - 1;
			break;
		}
		i++;
	}

	text.data = &contents.data[1];
	text.length--;

	// Set data.data to the contents.data text.length + 1 index
	data.data = &contents.data[text.length + 2];

	if (data.length == -1) exitMsg(RED "Error: No .data section" RESET, 1);
	if (data.length == 0) printf(YELLOW "Warning: .data section is empty" RESET);
}

/**
 * @brief Finds the labels and saves them to the labels struct
 **/
void indexLabels() {
	// Free labels if they already exist
	if (labels.length > 0) {
		for (int i = 0; i < labels.length; i++) free(labels.data[i].name);
		free(labels.data);
	}
	labels.length = 0;

	char* labelName = NULL;
	for (int i = 0; i < data.length; i++) {
		// If the last character is a colon, it's a label
		if (data.data[i][strlen(data.data[i]) - 2] == ':') {
			labelName = realloc(labelName, sizeof(char) * strlen(data.data[i]));
			strcpy(labelName, data.data[i]);
			labelName[strlen(labelName) - 2] = '\0';

			struct Label duplicateLabel = getLabel(labelName);
			int labelExists = duplicateLabel.position;

			if (labelExists != -1) {
				printf(YELLOW "Warning:" RESET " Label '" YELLOW "%s" RESET "' is already defined at " MAGENTA
							  "line %d" RESET " - Ignoring\n" RESET,
					   labelName, getGlobalLineNum(labelExists));
				printLine(labelExists);
				printf("\n");
				continue;
			}

			// Allocate memory for the label
			labels.data = realloc(labels.data, sizeof(struct Label) * labels.length + 1);

			// Set the label's name
			labels.data[labels.length].name = malloc(sizeof(char) * strlen(labelName));
			strcpy(labels.data[labels.length].name, labelName);

			// Set the label's line number
			labels.data[labels.length].position = i;
			labels.length++;
		}
	}
	free(labelName);
}

int getArgType(char* arg) {
	// Check if it's a register
	if (arg[0] == '$') {
		int reg = atoi(&arg[1]);
		if (reg >= 0 && reg < REGISTER_COUNT) return 0;
	}

	// Check if it's a number
	if (isdigit(arg[0]) || arg[0] == '-') {
		int num = atoi(arg);
		if (num >= 0 && num < 256) return 1;
	}

	// Check if it's a label
	for (int i = 0; i < labels.length; i++)
		if (strcmp(arg, labels.data[i].name) == 0) return 2;

	// Check if it's a data pointer
	if (arg[0] == '#') return 3;

	return -1;
}

char* argTypeToString(int type) {
	switch (type) {
	case 0:
		return "Register";
	case 1:
		return "Number";
	case 2:
		return "Label";
	case 3:
		return "Data Pointer";
	default:
		return "Unknown";
	}
}

void interpret(int* PC) {
	// Check if the line is a label
	if (isLabel(*PC)) return;

	char* line = data.data[*PC];

	if (line[0] == ';') return;
	if (trimStr(line)[0] == '\n') return;

	// Interpret the text section
	char* instruction = malloc(sizeof(char) * MAX_INSTRUCTION_LENGTH);
	strcpy(instruction, line);
	instruction = toLowerStr(strtok(instruction, " "));

	char** args = malloc(sizeof(char*));
	int argCount = 0;
	// Get the arguments and keep instruction.
	// Remove any \r or spaces or ',' from the arguments
	while ((args[argCount] = strtok(NULL, " ,\r\t\v\f\n\0")) != NULL) {
		argCount++;
		args = realloc(args, sizeof(char*) * (argCount + 1));
	}

	for (int i = 0; i < INSTRUCTION_COUNT; i++) {
		struct Instruction* inst = (struct Instruction*)instructions[i];
		if (strcmp(instruction, inst->name) == 0) {
			if (argCount != inst->argCount) {
				printf(RED "Error:" RESET " Invalid number of arguments for instruction " YELLOW "mov" RESET
						   " at " MAGENTA "line %d" RESET "\n",
					   getGlobalLineNum(*PC));
				printf("Expected " YELLOW "%d" RESET " argument/s, got " YELLOW "%d" RESET "\n", inst->argCount,
					   argCount);
				printLine(*PC);
				exit(1);
			}
			// Check if the arguments are valid
			for (int j = 0; j < argCount; j++) {
				int argType = getArgType(args[j]);
				if (argType != inst->argTypes[j]) {
					printf(RED "Error:" RESET " Invalid argument type for instruction " YELLOW "%s" RESET " at " MAGENTA
							   "line %d" RESET "\n",
						   inst->name, getGlobalLineNum(*PC));
					printf("Expected " YELLOW "%s" RESET ", got " YELLOW "%s" RESET "\n",
						   argTypeToString(inst->argTypes[j]), argTypeToString(argType));
					printLine(*PC);
					exit(1);
				}
			}

			((struct Instruction*)instructions[i])->func(PC, argCount, args);
			break;
		}
	}
	free(instruction);
}
// Get arguments from main
int main(int argc, char* argv[]) {
	argc = 2;
	argv[1] = "./tests/test.asm";
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
	// Index labels
	indexLabels();

	for (int PC = 0; PC < data.length; PC++) {
		printLine(PC);
		interpret(&PC);
		usleep(200000);
	}

	return 0;
}