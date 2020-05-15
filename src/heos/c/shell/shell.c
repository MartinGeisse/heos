
#include <string.h>
#include "../driver/console.h"
#include "shell.h"
#include "commands.h"

// --------------------------------------------------------------------------------------------------------------------
// value parsing
// --------------------------------------------------------------------------------------------------------------------

static int parseIntHelper(const char *text, int *destination) {

	// handle radix specifier, negative numbers and special cases
	int radix = 10;
	int sign = 1;
	if (text[0] == '0') {
		if (text[1] == 'x') {
			text += 2;
			radix = 16;
		} else if (text[1] == 'o') {
			text += 2;
			radix = 8;
		} else if (text[1] == 'b') {
			text += 2;
			radix = 2;
		} else if (text[1] == 0) {
			*destination = 0;
			return 1;
		} else {
			driver_console_println("leading 0 digits are forbidden in decimal integer literals to about ambiguity with respect to octal literals");
			return 0;
		}
	} else if (text[0] == '-') {
		text++;
		sign = -1;
	}

	// parse the actual number
	int magnitude = 0;
	while (1) {
		int digit;
		char c = *text;
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
			driver_console_formatln("invalid digit for radix %d: %c (%d)", radix, c, digit);
			return 0;
		}
		magnitude = radix * magnitude + digit;
		text++;
	}

	// success
	*destination = sign * magnitude;
	return 1;

}

static int parseValue(const char *text, const shell_ValuePattern *pattern, void *storage) {
    storage = ((char*)storage) + pattern->storageOffset;
    switch (pattern->type) {

        case shell_ValueType_string:
            *(char**)storage = text;
            return 1;

        case shell_ValueType_integer:
            return parseIntHelper(text, (int*)storage);

        default:
            driver_console_println("ERROR: unknown value pattern type");
            return 0;

    }
}

// --------------------------------------------------------------------------------------------------------------------
// segment splitting
// --------------------------------------------------------------------------------------------------------------------

static char *segments[16];
static int segmentCount;
static const shell_CommandPattern *commandPattern;

static int splitSegments(char *commandLine) {
    segmentCount = 0;
    int segmentStarted = 0;
    for (char *p = commandLine; *p != 0; p++) {
        if (*p == ' ') {
            segmentStarted = 0;
            *p = 0;
        } else if (!segmentStarted) {
            if (segmentCount == 16) {
                driver_console_println("ERROR: too many command line segments");
                return 0;
            }
            segmentStarted = 1;
            segments[segmentCount] = p;
            segmentCount++;
        }
    }
    return 1;
}

static void echoSplitCommandLine(void) {
	driver_console_print("executing: ");
	for (int i=0; i<segmentCount; i++) {
		driver_console_print("[");
		driver_console_print(segments[i]);
		driver_console_print("] ");
	}
	driver_console_println("");
}

// --------------------------------------------------------------------------------------------------------------------
// command implementation API
// --------------------------------------------------------------------------------------------------------------------

void foo(void) {
	// parse the command's arguments
	int minimumRepetitions = (commandPattern->repeatedArgument == NULL ? 0 : commandPattern->repeatedArgumentMinimumRepetitions);
	argumentCount = segmentCount - 1;
	if (argumentCount < commandPattern->fixedArgumentCount + minimumRepetitions) {
		driver_console_print("too few arguments for '");
		driver_console_print(commandName);
		driver_console_println("'. Usage:");
		shell_printSynopsis(commandPattern);
		driver_console_println("");
		return;
	}
	if (commandPattern->repeatedArgument == NULL && argumentCount > commandPattern->fixedArgumentCount) {
		driver_console_print("too many arguments for '");
		driver_console_print(commandName);
		driver_console_println("'. Usage:");
		shell_printSynopsis(commandPattern);
		driver_console_println("");
		return;
	}
	for (int i=0; i<commandPattern->fixedArgumentCount; i++) {
		const shell_ArgumentPattern *argumentPattern = (commandPattern->fixedArguments + i);
		const shell_ArgumentType *type = argumentPattern->type;
		if (!type->parser(segments[1 + i], parsedArguments + i)) {
			driver_console_formatln("syntax error in argument %d: expected %s. Usage:", (i + 2), type->name);
			shell_printSynopsis(commandPattern);
			driver_console_println("");
			return;
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
// command invocation API
// --------------------------------------------------------------------------------------------------------------------

void shell_executeCommandLine(char *commandLine) {

    // split into segments
    if (!splitSegments(commandLine)) {
        return;
    }

	// skip empty lines
	if (segmentCount == 0) {
		return;
	}

	// show the command to execute
	echoSplitCommandLine();

	// find a matching command
	const char *commandName = segments[0];
	commandPattern = NULL;
	for (int i=0; i<shell_commandPatternCount; i++) {
		const shell_CommandPattern *tryCommandPattern = shell_commandPatterns + i;
		if (strcmp(commandName, tryCommandPattern->name) == 0) {
			commandPattern = tryCommandPattern;
			break;
		}
	}
	if (commandPattern == NULL) {
		driver_console_print("unknown command: ");
		driver_console_println(commandName);
		return;
	}

	// execute the command's callback
	commandPattern->callback();

}


// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO
// TODO

// --------------------------------------------------------------------------------------------------------------------
// argument parsing
// --------------------------------------------------------------------------------------------------------------------

static shell_ParsedArgument parsedArguments[16];
static int argumentCount;

static int parseString(const char *segment, shell_ParsedArgument *destination) {
	destination->properties[0].asText = segment;
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

// --------------------------------------------------------------------------------------------------------------------
// main command execution logic
// --------------------------------------------------------------------------------------------------------------------

void shell_executeCommandLine(char *commandLine) {

    // split into segments
    if (!splitSegments(commandLine)) {
        return;
    }

	// skip empty lines
	if (segmentCount == 0) {
		return;
	}

	// show the command to execute
	echoSplitCommandLine();

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
		driver_console_print("unknown command: ");
		driver_console_println(commandName);
		return;
	}

	// parse the command's arguments
	int minimumRepetitions = (commandPattern->repeatedArgument == NULL ? 0 : commandPattern->repeatedArgumentMinimumRepetitions);
	argumentCount = segmentCount - 1;
	if (argumentCount < commandPattern->fixedArgumentCount + minimumRepetitions) {
		driver_console_print("too few arguments for '");
		driver_console_print(commandName);
		driver_console_println("'. Usage:");
		shell_printSynopsis(commandPattern);
		driver_console_println("");
		return;
	}
	if (commandPattern->repeatedArgument == NULL && argumentCount > commandPattern->fixedArgumentCount) {
		driver_console_print("too many arguments for '");
		driver_console_print(commandName);
		driver_console_println("'. Usage:");
		shell_printSynopsis(commandPattern);
		driver_console_println("");
		return;
	}
	for (int i=0; i<commandPattern->fixedArgumentCount; i++) {
		const shell_ArgumentPattern *argumentPattern = (commandPattern->fixedArguments + i);
		const shell_ArgumentType *type = argumentPattern->type;
		if (!type->parser(segments[1 + i], parsedArguments + i)) {
			driver_console_formatln("syntax error in argument %d: expected %s. Usage:", (i + 2), type->name);
			shell_printSynopsis(commandPattern);
			driver_console_println("");
			return;
		}
	}

	// execute the command's callback
	commandPattern->callback(argumentCount, parsedArguments);

}

void shell_printSynopsis(const shell_CommandPattern *commandPattern) {
	driver_console_print(commandPattern->name);
	for (int j=0; j<commandPattern->fixedArgumentCount; j++) {
		const shell_ArgumentPattern *argumentPattern = commandPattern->fixedArguments + j;
		driver_console_print(" <");
		driver_console_print(argumentPattern->name);
		if (argumentPattern->type->name != NULL) {
			driver_console_print(":");
			driver_console_print(argumentPattern->type->name);
		}
		driver_console_print(">");
	}
	if (commandPattern->repeatedArgument != NULL) {
		const shell_ArgumentPattern *argumentPattern = commandPattern->repeatedArgument;
		driver_console_print(" (<");
		driver_console_print(argumentPattern->name);
		if (argumentPattern->type->name != NULL) {
			driver_console_print(":");
			driver_console_print(argumentPattern->type->name);
		}
		if (commandPattern->repeatedArgumentMinimumRepetitions == 0) {
			driver_console_print(">)*");
		} else if (commandPattern->repeatedArgumentMinimumRepetitions == 1) {
			driver_console_print(">)+");
		} else {
			driver_console_format(">)[%d+]", commandPattern->repeatedArgumentMinimumRepetitions);
		}
	}
}
