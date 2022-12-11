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
There is also a --debug flag that will print out the registers and the current instruction.

```bash
# Creating your own instructions
Well... open up [instructions.c](./instructions.c)   
Instructions are defined like this:
```c
void newInstructionFunc(int* PC char* args[])
```
And you also need to define the instruction info:
```c
struct Instruction newInstruction = {"newInstruction", (int[]){args...}, (void*)newInstructionFunc};
```
Finally, increment `INSTRUCTION_COUNT` at the end of the file.
and append your instruction to the `instructions` array.

## Example instructions:

This one takes in a single argument of the type label and jumps to that label. 
```c
void jmpFunc(int* PC, int args[]) { *PC = args[0]; }
struct Instruction jmp = {"jmp", 1, (int[]){2}, (void*)jmpFunc};
```
  
This one takes in two arguments. The first one is the type register and the second one is the type number. It then moves the number into the register.
```c
void movFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	registers[reg1] = registers[reg2];
}
struct Instruction mov = {"mov", 2, (int[]){0, 0}, (void*)movFunc};
```

There are 4 types of arguments types:
- `0` - Register
- `1` - Number
- `2` - Label
- `3` - Data Pointer (not implemented yet) 

Depending on type of argument it will give you a different value.
- `0` - Register index
- `1` - Number
- `2` - (Program instruction PC index) - 1 
- `3` - Data pointer index (not implemented yet) 

That's it.
Okay Have fun!