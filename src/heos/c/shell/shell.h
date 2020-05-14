
#ifndef HDR__SHELL_SHELL_H__
#define HDR__SHELL_SHELL_H__

typedef union {
	int asInt;
	void *asPointer;
} shell_ParsedArgumentProperty;

typedef struct {
	shell_ParsedArgumentProperty properties[4];
} shell_ParsedArgument;

typedef struct {
	const char *name;
	int (*parser)(const char *segment, shell_ParsedArgument *destination);
} shell_ArgumentType;

typedef struct {
	const char *name;
	const shell_ArgumentType *type;
} shell_ArgumentPattern;

typedef struct {
	const char *name;
	int fixedArgumentCount;
	const shell_ArgumentPattern *fixedArguments;
	const shell_ArgumentPattern *repeatedArgument;
	int repeatedArgumentMinimumRepetitions;
	void (*callback)(int argumentCount, const shell_ParsedArgument *destination);
} shell_CommandPattern;

extern shell_ArgumentType shell_stringArgumentType;
extern shell_ArgumentType shell_intArgumentType;

int shell_parseCommandLine(char *commandLine);
void shell_executeCommandLine();
void shell_printSynopsis(const shell_CommandPattern *commandPattern);

#endif
