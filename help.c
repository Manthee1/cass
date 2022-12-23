
#include "headers.h"
#include "_utils.c"
#include "globals.c"

#define OPTION_COUNT 8
void printHelp();
struct HelpItem {
	char* name;
	char shortName;
	char* description;
	// validation function pointer
	int (*validate)();
};

void validateOptionValueInt(int argc, char* argv[], int index, int min, int max, char* argName) {
	// if value begins with a '-', it is another argument
	if (index + 1 >= argc || argv[index + 1][0] == '-') {
		printf("Expected an argument for %s\n", argName);
		exit(1);
	}

	int count = atoi(argv[index + 1]);
	if (min == 0 && count < 1) {
		printf("Expected a positive integer for %s\n", argName);
		exit(1);
	}
	if (max == -1 && count < min) {
		printf("Expected a integer greater than %d for %s\n", min, argName);
		exit(1);
	}
	if (max != -1 && (count < min || count > max)) {
		printf("Expected a value between %d and %d for %s\n", min, max, argName);
		exit(1);
	}
}

void validateOptionValueString(int argc, char* argv[], int index, char* argName) {
	// if value begins with a '-', it is another argument
	if (index + 1 >= argc || argv[index + 1][0] == '-') {
		printf("Expected an argument for %s\n", argName);
		exit(1);
	}
}

int printVersion() {
	printf("Version: %s\n", VERSION);
	exit(0);
}
int verboseValidate(int argc, char* argv[], int* index) {
	verbose = 1;
	return 0;
}

int debugValidate(int argc, char* argv[], int* index) {
	debug = 1;
	return 0;
}

int strictValidate(int argc, char* argv[], int* index) {
	strict = 1;
	return 0;
}

int registersValidate(int argc, char* argv[], int* index) {
	validateOptionValueInt(argc, argv, *index, 1, MAX_REGISTER_COUNT, argv[*index]);
	registerCount = atoi(argv[*index + 1]);
	(*index)++;
	return 0;
}

int registerSizeValidate(int argc, char* argv[], int* index) {
	validateOptionValueInt(argc, argv, *index, 1, MAX_REGISTER_SIZE, argv[*index]);
	registerSize = atoi(argv[*index + 1]);
	(*index)++;
	return 0;
}

int speedValidate(int argc, char* argv[], int* index) {
	validateOptionValueInt(argc, argv, *index, 1, -1, argv[*index]);
	speed = atoi(argv[*index + 1]);
	if (speed >= 100) printException("Speed larger or equal to 100 is considered as instant execution\n", INFO, -1);
	(*index)++;
	return 0;
}

// Help gets printHelp functin
struct HelpItem helpItems[OPTION_COUNT] = {
	{"help", 'h', "Display this help message", (int (*)())printHelp},
	{"verbose", 'v', "Display verbose output", verboseValidate},
	{"version", 'V', "Display version information", (int (*)())printVersion},
	{"debug", 'd', "Display debug output", debugValidate},
	{"strict", 0, "Exit with an error if there are any runtime warnings", strictValidate},
	{"registers", 'r', "How many registers the program has ($0 is not counted)", registersValidate},
	{"register-size", 'S', "How much bytes a register can hold", registerSizeValidate},
	{"speed", 's', "How many instructions to execute per second (max 100)", speedValidate},
};

void validateOptions(int argc, char* argv[]) {
	for (int i = 2; i < argc; i++) {
		int found = 0;
		char* argName = argv[i];

		// Remove the '-' from the argument name 2x if it has 2
		int isDoubleDash = 0;
		if (argName[0] == '-') argName++;
		if (argName[0] == '-') {
			argName++;
			isDoubleDash = 1;
		}
		for (int j = 0; j < OPTION_COUNT; j++) {
			if ((isDoubleDash && strcmp(argName, helpItems[j].name) != 0) ||
				(!isDoubleDash && (helpItems[j].shortName == 0 || argName[0] != helpItems[j].shortName)))
				continue;
			if (helpItems[j].validate != NULL) helpItems[j].validate(argc, argv, &i);

			found = 1;
			break;
		}

		if (!found) {
			printf("Unknown option '%s'\n", argv[i]);
			exit(1);
		}
	}
}

void printHelp() {
	printf("Usage: cass <filename> [options]\n");
	for (int i = 0; i < OPTION_COUNT; i++) {
		printf("  --%s", helpItems[i].name);
		if (helpItems[i].shortName != 0) printf(", -%c", helpItems[i].shortName);

		// Calculate the amount of tabs to print depending on the length of the name
		int tabs = 2 - (helpItems[i].name[0] == 'r' ? 1 : 0);
		for (int j = 0; j < tabs; j++) printf("\t");

		printf("%s\n", helpItems[i].description);
	}
	exit(0);
}

// if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
// 	debug = 1;
// 	continue;
// }
// if (strcmp(argv[i], "--registers") == 0 || strcmp(argv[i], "-r") == 0) {
// 	validateArgumentValue(argc, argv, i, argv[i]);
// 	registerCount = atoi(argv[i + 1]);
// 	if (registerCount > MAX_REGISTER_COUNT) {
// 		printf("Register count cannot be larger than %d\n", MAX_REGISTER_COUNT);
// 		return 1;
// 	}
// 	i++;
// 	continue;
// }
// if (strcmp(argv[i], "--register-size") == 0 || strcmp(argv[i], "-s") == 0) {
// 	validateArgumentValue(argc, argv, i, argv[i]);
// 	registerSize = atoi(argv[i + 1]);
// 	if (registerSize > MAX_REGISTER_SIZE) {
// 		printf("Register size cannot be larger than %d\n", MAX_REGISTER_SIZE);
// 		return 1;
// 	}
// 	i++;
// 	continue;
// }
// if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
// 	verbose = 1;
// 	continue;
// }
// if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
// 	printf("Cass version %s\n", VERSION);
// 	return 0;
// }
// if (strcmp(argv[i], "--speed") == 0 || strcmp(argv[i], "-S") == 0) {
// 	validateArgumentValue(argc, argv, i, argv[i]);
// 	speed = atoi(argv[i + 1]);
// 	if (speed >= 100) printException("Speed larger or equal to 100 is considered as instant execution\n", INFO, -1);
// 	i++;
// 	continue;
// }
// if (strcmp(argv[i], "--strict") == 0) {
// 	strict = 1;
// 	continue;
// }
// printf("Unknown argument '%s'\n", argv[i]);
// return 1;