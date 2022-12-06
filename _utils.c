#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

void exitMsg(char* message, int code) {
	printf("%s\n", message);
	exit(code);
}
char* toLowerStr(char* str) {
	char* newStr = malloc(sizeof(char) * strlen(str));
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] >= 'A' && str[i] <= 'Z')
			newStr[i] = str[i] + 32;
		else
			newStr[i] = str[i];
	}
	return newStr;
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

int getFileLength(FILE* file) {
	// Save pointer
	fpos_t pos;
	fgetpos(file, &pos);
	// Get length
	int length = 0;
	char c;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') length++;
	}
	// Reset pointer
	fsetpos(file, &pos);

	return length;
}

int wrap(int num, int min, int max) {
	if (num < min) return max;
	if (num > max) return min;
	return num;
}