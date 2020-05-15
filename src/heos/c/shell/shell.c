
#include <string.h>
#include "../driver/console.h"
#include "shell.h"
#include "commands.h"

// --------------------------------------------------------------------------------------------------------------------
// value types
// --------------------------------------------------------------------------------------------------------------------

const char *shell_valueTypeNames[shell_maxValueType] = {
    "string",
    "integer",
};

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

typedef enum {
    SegmentKind_argument,
    SegmentKind_optionDelimiter,
    SegmentKind_shortOptions,
    SegmentKind_longOption,
    SegmentKind_invalid,
} SegmentKind;

static SegmentKind determineSegmentKind(const char *segment) {
    if (segment[0] != '-' || segment[1] == 0) {
        return SegmentKind_argument;
    }
    if (segment[1] != '-') {
        return SegmentKind_shortOptions;
    }
    if (segment[2] == 0) {
        return SegmentKind_optionDelimiter;
    }
    if (segment[2] == '-') {
        // nothing starts with "---"
        return SegmentKind_invalid;
    }
    return SegmentKind_longOption;
}

int shell_processOptionsAndArguments(void *storage) {

    // prepare fixed argument storage
    shell_ValuePattern *nextFixedArgumentPattern = commandPattern->fixedArguments;
    if (nextFixedArgumentPattern->displayName == NULL) {
        nextFixedArgumentPattern = NULL;
    }

    // set all option flags to 0
    if (commandPattern->options != NULL) {
        for (shell_OptionPattern *option = commandPattern->options; option->name != NULL; option++) {
            if (option->flagOffset >= 0) {
                *(((char*)storage) + option->flagOffset) = 0;
            }
        }
    }

    int optionDelimiterSeen = 0;
    for (int i = 0; i < segmentCount; i++) {
        const char *segment = segments[i];
        SegmentKind kind = optionDelimiterSeen ? SegmentKind_argument : determineSegmentKind(segment);
        switch (kind) {

            case SegmentKind_argument:
                if (nextFixedArgumentPattern == NULL) {
                    if (commandPattern->repeatedArgument == NULL) {
                        driver_console_print("unexpected command argument: ");
                        driver_console_println(segment);
                        return 0;
                    } else {
                        // TODO repeated argument
                    }
                } else {
                    // TODO fixed argument

                    nextFixedArgumentPattern++;
                    if (nextFixedArgumentPattern->displayName == NULL) {
                        nextFixedArgumentPattern = NULL;
                    }
                }
                break;

            case SegmentKind_optionDelimiter:
                optionDelimiterSeen = 1;
                break;

            case SegmentKind_shortOptions:
                for (char *p = segment + 1; *p != 0; p++) {

                    // TODO
                }
                break;

            case SegmentKind_longOption:
                // TODO
                break;

            case SegmentKind_invalid:
           		driver_console_print("invalid command segment: ");
           		driver_console_println(segment);
           		return 0;

        }
    }
    if (nextFixedArgumentPattern != NULL) {
        driver_console_print("too few arguments, expected: ");
        driver_console_println(nextFixedArgumentPattern->displayName);
        return 0;
    }
    return 1;
}

void foo(void) {
	// parse the command's arguments
	int minimumRepetitions = (commandPattern->repeatedArgument == NULL ? 0 : commandPattern->repeatedArgumentMinimumRepetitions);
	argumentCount = segmentCount - 1;
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

// --------------------------------------------------------------------------------------------------------------------
// command meta-data API
// --------------------------------------------------------------------------------------------------------------------

void shell_printSynopsis(const shell_CommandPattern *commandPattern) {

    // name
	driver_console_print(commandPattern->name);

	// options
	if (commandPattern->options != NULL && commandPattern->options->name != NULL) {
	    driver_console_print(" [options] ");
	} else {
	    driver_console_print(" ");
	}

	// fixed arguments
	if (commandPattern->fixedArguments != NULL) {
        for (shell_ValuePattern *argumentPattern = commandPattern->fixedArguments;
                argumentPattern->displayName != NULL;
                argumentPattern++) {
            driver_console_print(" <");
            driver_console_print(argumentPattern->displayName);
            if (argumentPattern->type != shell_ValueType_string) { // don't say "string" when we don't know better
                driver_console_print(":");
                driver_console_print(shell_valueTypeNames[argumentPattern->type]);
            }
            driver_console_print(">");
        }
	}

	// repeated arguments
	if (commandPattern->repeatedArgument != NULL) {
		const shell_ArgumentPattern *argumentPattern = commandPattern->repeatedArgument;
		driver_console_print(" <");
		driver_console_print(argumentPattern->displayName);
        if (argumentPattern->type != shell_ValueType_string) { // don't say "string" when we don't know better
            driver_console_print(":");
            driver_console_print(shell_valueTypeNames[argumentPattern->type]);
        }
        driver_console_print(" ...>");
	}

	driver_console_println("");
}

shell_OptionPattern *shell_findShortOption(const shell_CommandPattern *commandPattern, char name) {
    // TODO
}

shell_OptionPattern *shell_findOption(const shell_CommandPattern *commandPattern, const char *name) {
    // TODO
}
