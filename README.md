# What is this?
This? Oh.. Well... This is a interpreter for my wired assembly like instructions.
I got bored. So I made this. I don't know why. I just did. It was definitely a waste of time. And now I'm sharing it with you. So you can waste your time too.

I think I bested my self when it comes to unorganized code. This is a mess. I'm sorry.

# How to use it?
Compile [cass.c](./cass.c) with gcc. Then run it with the file you want to interpret as the first argument. For example:
```bash
gcc cass.c -o cass
./cass test.asm
```
# Creating your own instructions
Well... open up [instructions.c](./instructions.c)   
Instructions are defined like this:
```c
void newInstructionFunc(int* PC, int argCount, char* args[])
```
And you also need to define the instruction info:
```c
struct Instruction newInstruction = {"newInstruction", argCount, (int[]){args...}, (void*)newInstructionFunc};
```
Finally, increment `INSTRUCTION_COUNT` at the end of the file.
and append your instruction to the `instructions` array.

## Example instructions:

This one takes in a single argument of the type label and jumps to that label. 
```c
void jmpFunc(int* PC, int argCount, char* args[]) {
	int label = getLabelPosition(args[0]);
	*PC = label;
}
struct Instruction jmp = {"jmp", 1, (int[]){2}, (void*)jmpFunc};
```
 
  
This one takes in two arguments. The first one is the type register and the second one is the type number. It then moves the number into the register.
```c
void movcFunc(int* PC, int argCount, char* args[]) {
	int reg = args[0][1] - '0';
	int value = atoi(args[1]);
	registers[reg] = value;
}
struct Instruction movc = {"movc", 2, (int[]){0, 1}, (void*)movcFunc};
```

There are 4 types of arguments types:
- `0` - Register
- `1` - Number
- `2` - Label
- `3` - Data Pointer

That's it.
Okay Have fun!