# What is this?
This? Oh.. Well... This is a interpreter for my wired assembly like instructions.
I got bored. So I made this. I don't know why. I just did. It was definitely a waste of time. And now I'm sharing it with you. So you can waste your time too.

I think I bested my self when it comes to unorganized code. This is a mess. I'm sorry.

### **Please report any bugs you find!**
This is a complex shit show and I'm too lazy to write tests.  
I'm like YandareDev but like... less spaghetti.

# How to use it?
Compile [cass.c](./cass.c) with gcc. Then run it with the file you want to interpret as the first argument. For example:
```bash
gcc cass.c -o cass
./cass <file>
```
## Arguments
- `-d` or `--debug` - Shows you a nice debug screen
- `-r <amount>` or `--registers <amount>` - Specifies max amount of registers ($0 is not counted)
- `-S <amount>` or `--register-size <amount>` - Specifies how much bits a register can hold
- `-s <amount>` or `--speed <amount>` - How many instructions to execute per second (max 100)
- `-V` or `--version` - Print the version of the program
- `-v` or `--verbose` - Print all the normally ignored warnings and errors
- `--strict` - Exit with an error if there are any runtime warnings
- `-h` or `--help` - Prints the help message

# Creating your own instructions
Well... open up [instructions.c](./instructions.c)   
Instructions are defined like this:
```c
void newInstructionFunc(int* PC char* args[])
```
And you also need to define the instruction info:
```c
struct Instruction newInstruction = {"newInstruction", argCount, (int[]){...argType}, (void*)newInstructionFunc};
```
Finally, increment `INSTRUCTION_COUNT` at the end of the file.
and append your instruction to the `instructions` array.

## Example instructions:

This one takes in a single argument of the type label and jumps to that label. 
```c
void jmpFunc(int* PC, int args[]) { *PC = args[0]; }
struct Instruction jmp = {"jmp", 1, (int[]){LABEL}, (void*)jmpFunc};
```
  
This one takes in two arguments. The first one is the type register and the second one is the type number. It then moves the number into the register.
```c
void movFunc(int* PC, int args[]) {
	int reg1 = args[0];
	int reg2 = args[1];
	registers[reg1] = registers[reg2];
}
struct Instruction mov = {"mov", 2, (int[]){REGISTER, NUMBER}, (void*)movFunc};
```

There are 4 types of arguments types:
- `0` or `REGISTER` - Register
- `1` or `NUMBER` - Number
- `2` or `LABEL` - Label
- `3` or `DATA_POINTER` - Data pointer

Depending on type of argument it will give you a different value.
###  `REGISTER` - Register index
```c
	// The register is the index of the register
	int regIndex = args[n];
	int registerValue = registers[regIndex]; // The value of the register
```

### `NUMBER` - Number
```c
	// The number is already an int
	int num = args[n];
```
### `LABEL` - PC (Program instruction) index 
```c
	// The label is the PC index of the label
	int labelIndex = args[n];
	// You can set *PC to this value to jump to the label
	*PC = labelIndex;
```
### `DATA_POINTER` - Data pointer index

```c
int dataPointerIndex = args[n];

//You can get the data from the data list with these functions:
char* dataString = getDataString(dataList, dataPointerIndex);
int dataInt = getDataInt(dataList, dataPointerIndex);
```

That's it.
Okay Have fun!