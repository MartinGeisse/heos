
#include <string.h>
#include "../console/console.h"
#include "shell.h"
#include "commands.h"

static char *segments[16];
static int segmentCount;
static shell_ParsedArgument parsedArguments[16];
static int argumentCount;

static int parseString(const char *segment, shell_ParsedArgument *destination) {
	destination->properties[0].asPointer = segment;
}

static int parseInt(const char *segment, shell_ParsedArgument *destination) {

	// handle radix specifier, negative numbers and special cases
	int radix = 10;
	int sign = 1;
	if (segment[0] == '0') {
		if (segment[1] == 'x') {
			segment += 2;
			radix = 16;
		} else if (segment[1] == 'o') {
			segment += 2;
			radix = 8;
		} else if (segment[1] == 'b') {
			segment += 2;
			radix = 2;
		} else if (segment[1] == 0) {
			destination->properties[0].asInt = 0;
			return 1;
		} else {
			console_printLine("leading 0 digits are forbidden in decimal integer literals to about ambiguity with respect to octal literals");
			return 0;
		}
	} else if (segment[0] == '-') {
		segment++;
		sign = -1;
	}

	// parse the actual number
	int magnitude = 0;
	while (1) {
		int digit;
		char c = *segment;
		if (c == 0) {
			break;
		} else if (c >= '0' && c <= '9') {
			digit = (c - '0');
		} else if (c >= 'a' && c <= 'z') {
			digit = (c - 'a' + 10);
		} else if (c >= 'A' && c <= 'Z') {
			digit = (c - 'A' + 10);
		} else {
			return 0;
		}
		if (digit >= radix) {
			console_formatLine("invalid digit for radix %d: %c (%d)", radix, c, digit);
			return 0;
		}
		magnitude = radix * magnitude + digit;
		segment++;
	}

	// success
	destination->properties[0].asInt = sign * magnitude;
	return 1;

}

shell_ArgumentType shell_stringArgumentType = {
	.name = NULL,
	.parser = parseString,
};
shell_ArgumentType shell_intArgumentType = {
	.name = "int",
	.parser = parseInt,
};

int shell_parseCommandLine(char *commandLine) {
    segmentCount = 0;
    int segmentStarted = 0;
    for (char *p = commandLine; *p != 0; p++) {
        if (*p == ' ') {
            segmentStarted = 0;
            *p = 0;
        } else if (!segmentStarted) {
            if (segmentCount == 16) {
                console_printLine("ERROR: too many command line segments");
                return 0;
            }
            segmentStarted = 1;
            segments[segmentCount] = p;
            segmentCount++;
        }
    }
    return 1;
}

void shell_executeCommandLine() {

	// skip empty lines
	if (segmentCount == 0) {
		console_printLine("");
		return;
	}

	// show the command to execute
	console_printLine("");
	console_print("* ");
	for (int i=0; i<segmentCount; i++) {
		console_print(segments[i]);
		console_print(" ");
	}
	console_printLine("");

	// find a matching command pattern
	const char *commandName = segments[0];
	const shell_CommandPattern *commandPattern = NULL;
	for (int i=0; i<shell_commandPatternCount; i++) {
		const shell_CommandPattern *tryCommandPattern = shell_commandPatterns + i;
		if (strcmp(commandName, tryCommandPattern->name) == 0) {
			commandPattern = tryCommandPattern;
			break;
		}
	}
	if (commandPattern == NULL) {
		console_print("unknown command: ");
		console_printLine(commandName);
		return;
	}

	// parse the command's arguments
	int minimumRepetitions = (commandPattern->repeatedArgument == NULL ? 0 : commandPattern->repeatedArgumentMinimumRepetitions);
	argumentCount = segmentCount - 1;
	if (argumentCount < commandPattern->fixedArgumentCount + minimumRepetitions) {
		console_print("too few arguments for '");
		console_print(commandName);
		console_printLine("'. Usage:");
		shell_printSynopsis(commandPattern);
		console_printLine("");
		return;
	}
	if (commandPattern->repeatedArgument == NULL && argumentCount > commandPattern->fixedArgumentCount) {
		console_print("too many arguments for '");
		console_print(commandName);
		console_printLine("'. Usage:");
		shell_printSynopsis(commandPattern);
		console_printLine("");
		return;
	}
	for (int i=0; i<commandPattern->fixedArgumentCount; i++) {
		const shell_ArgumentPattern *argumentPattern = (commandPattern->fixedArguments + i);
		const shell_ArgumentType *type = argumentPattern->type;
		if (!type->parser(segments[1 + i], parsedArguments + i)) {
			console_formatLine("syntax error in argument %d: expected %s. Usage:", (i + 2), type->name);
			shell_printSynopsis(commandPattern);
			console_printLine("");
			return;
		}
	}

	// execute the command's callback
	commandPattern->callback(argumentCount, parsedArguments);

}

void shell_printSynopsis(shell_CommandPattern *commandPattern) {
	console_print(commandPattern->name);
	for (int j=0; j<commandPattern->fixedArgumentCount; j++) {
		const shell_ArgumentPattern *argumentPattern = commandPattern->fixedArguments + j;
		console_print(" <");
		console_print(argumentPattern->name);
		if (argumentPattern->type->name != NULL) {
			console_print(":");
			console_print(argumentPattern->type->name);
		}
		console_print(">");
	}
	if (commandPattern->repeatedArgument != NULL) {
		const shell_ArgumentPattern *argumentPattern = commandPattern->repeatedArgument;
		console_print(" (<");
		console_print(argumentPattern->name);
		if (argumentPattern->type->name != NULL) {
			console_print(":");
			console_print(argumentPattern->type->name);
		}
		if (commandPattern->repeatedArgumentMinimumRepetitions == 0) {
			console_print(">)*");
		} else if (commandPattern->repeatedArgumentMinimumRepetitions == 1) {
			console_print(">)+");
		} else {
			console_format(">)[%d+]", commandPattern->repeatedArgumentMinimumRepetitions);
		}
	}
}
