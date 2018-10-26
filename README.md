# minimal-c-compiler
A "minimal" C compiler implementation developed for CS 4318 - Compiler Construction.
## Files Required to Build Parser:
* driver.c
* cgen.c - generates assembly code for .mC programs and creates an intermediate activation record structure for navigating function calls
* cgen.h
* errors.c - error handler for semantic analysis errors
* errors.h
* parser.y - parser 
* scanner.l - token scanner
* symtable.c - generates the symbol tables for the .mC program
* symtable.h
* tree.c - generates the AST structure for the program
* tree.h
* makefile (optional)

## Target Information: 
* MIPs and the MARs simulator.
* Compiled with GCC on Ubuntu.

## Running mcc:
Runs the parser against the input file. No message means it passed.

```./mcc < inputFile.mC```

Runs the parser against the input file and prints the AST.

```./mcc -p < inputFile.mC```

Runs the parser against the input file and prints a global symbol table and any local symbol tables.

```./mcc -s < inputFile.mC```

Generates code for the input file and prints the Activation Records list for the file.

```./mcc -a < inputFile.mC```

Generates code for the input file as a out.asm file. This file is output in the mcc executable directory.

```./mcc -c < inputFile.mC```

## Notes:
* if-else implementation incomplete. Generates the statements that test the relational expressions and the statements that generate the jump to label. I did write a function for generating labels. Test something like: ```if (x == 3)``` to see what is unfinished.
* While loop: unimplemented.
* output() prints immediate values, but doesn't print values stored in registers.
* Line numbers in error messages are off.
