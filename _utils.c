#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

void exitMsg(char* message, int code) {
	printf("%s\n", message);
	exit(code);
}
void clear() { system("clear || cls"); }

void toLowerStr(char* str) {
	for (int i = 0; i < strlen(str); i++) str[i] = (str[i] >= 'A' && str[i] <= 'Z') ? str[i] + 32 : str[i];
}

char* trimStr(char* str) {
	int start = 0;
	int end = strlen(str) - 1;
	while (str[start] == ' ' || str[start] == '\t') start++;
	while (str[end] == ' ' || str[end] == '\t') end--;
	char* newStr = malloc(sizeof(char) * (end - start + 2));
	for (int i = start; i <= end; i++) newStr[i - start] = str[i];
	newStr[end - start + 1] = '\0';
	return newStr;
}

int getFileLengthLines(FILE* file) {
	// Save pointer
	fpos_t pos;
	fgetpos(file, &pos);

	// Get amount of lines
	int length = 0;
	char c;
	while ((c = fgetc(file)) != EOF)
		if (c == '\n') length++;

	// Reset pointer
	fsetpos(file, &pos);

	return length;
}

int getFileLength(FILE* file) {
	// Save pointer
	fpos_t pos;
	fgetpos(file, &pos);

	// Get amount of characters
	fseek(file, 0, SEEK_END);
	int length = ftell(file);

	// Reset pointer
	fsetpos(file, &pos);

	return length;
}

int wrap(int num, int min, int max) {
	if (num < min) return max;
	if (num > max) return min;
	return num;
}