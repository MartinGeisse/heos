
#include "../driver/terminal.h"
#include "shell.h"
#include "commands.h"
#include "../string.h"

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

static int parseIntHelper(const char *displayName, const char *text, int *destination) {

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
		    driver_terminal_printString("syntax error in <");
		    driver_terminal_printString(displayName);
			driver_terminal_printlnString(">: leading 0 digits are forbidden in decimal integer literals to about ambiguity with respect to octal literals");
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
			driver_terminal_printString("syntax error in <");
            driver_terminal_printString(displayName);
            driver_terminal_printString(">: invalid digit for radix ");
            driver_terminal_printInt(radix);
            driver_terminal_printString(": ");
            driver_terminal_printChar(c);
            driver_terminal_printString(" (");
            driver_terminal_printInt(digit);
            driver_terminal_printlnChar(')');
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
            *(const char**)storage = text;
            return 1;

        case shell_ValueType_integer:
            return parseIntHelper(pattern->displayName, text, (int*)storage);

        default:
            driver_terminal_printlnString("ERROR: unknown value pattern type");
            return 0;

    }
}

// --------------------------------------------------------------------------------------------------------------------
// segment splitting
// --------------------------------------------------------------------------------------------------------------------

static const char *segments[16];
static int segmentCount;
static int repeatedSegmentsConsumed;
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
                driver_terminal_printlnString("ERROR: too many command line segments");
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
	driver_terminal_printString("executing: ");
	for (int i=0; i<segmentCount; i++) {
		driver_terminal_printChar('[');
		driver_terminal_printString(segments[i]);
		driver_terminal_printString("] ");
	}
	driver_terminal_println();
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

int shell_processOptionsAndFixedArguments(void *storage) {

    // We use this index to change the list of segments while processing it, so only a list of repeated arguments
    // remains for shell_processRepeatedArgument().
    int repeatedArgumentsWritten = 0;

    // prepare fixed argument storage
    const shell_ValuePattern *nextFixedArgumentPattern = commandPattern->fixedArguments;
    if (nextFixedArgumentPattern != NULL && nextFixedArgumentPattern->displayName == NULL) {
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
    for (int i = 1; i < segmentCount; i++) {
        const char *segment = segments[i];
        SegmentKind kind = optionDelimiterSeen ? SegmentKind_argument : determineSegmentKind(segment);
        switch (kind) {

            case SegmentKind_argument:
                if (nextFixedArgumentPattern == NULL) {
                    if (commandPattern->repeatedArguments == NULL) {
                        driver_terminal_printString("unexpected command argument: ");
                        driver_terminal_printlnString(segment);
                        return 0;
                    } else {
                        segments[repeatedArgumentsWritten] = segment;
                        repeatedArgumentsWritten++;
                    }
                } else {
                    if (!parseValue(segment, nextFixedArgumentPattern, storage)) {
                        return 0;
                    }
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
                for (const char *p = segment + 1; *p != 0; p++) {
                    shell_OptionPattern *optionPattern = shell_findShortOption(commandPattern, *p);
                    if (optionPattern == NULL) {
                        driver_terminal_printString("unknown option: -");
                        driver_terminal_printChar(*p);
                        driver_terminal_println();
                        return 0;
                    }
                    if (optionPattern->flagOffset >= 0) {
                        *(((char*)storage) + optionPattern->flagOffset) = 1;
                    }
                    if (optionPattern->argument != NULL) {
                        if (p[1] != 0) {
                            driver_terminal_printString("found option with argument in the middle of multiple short options: -");
                            driver_terminal_printChar(*p);
                            driver_terminal_println();
                            return 0;
                        }
                        if (i == segmentCount - 1) {
                            driver_terminal_printString("missing argument for option: -");
                            driver_terminal_printChar(*p);
                            driver_terminal_println();
                            return 0;
                        }
                        if (!parseValue(segments[i + 1], optionPattern->argument, storage)) {
                            return 0;
                        }
                        i++;
                    }
                }
                break;

            case SegmentKind_longOption: {
                shell_OptionPattern *optionPattern = shell_findOption(commandPattern, segment + 2);
                if (optionPattern == NULL) {
                    driver_terminal_printString("unknown option: ");
                    driver_terminal_printlnString(segment);
                    return 0;
                }
                if (optionPattern->flagOffset >= 0) {
                    *(((char*)storage) + optionPattern->flagOffset) = 1;
                }
                if (optionPattern->argument != NULL) {
                    if (i == segmentCount - 1) {
                        driver_terminal_printString("missing argument for option: ");
                        driver_terminal_printlnString(segment);
                        return 0;
                    }
                    if (!parseValue(segments[i + 1], optionPattern->argument, storage)) {
                        return 0;
                    }
                    i++;
                }
                break;
            }

            case SegmentKind_invalid:
           		driver_terminal_printString("invalid command segment: ");
           		driver_terminal_printlnString(segment);
           		return 0;

        }
    }
    if (nextFixedArgumentPattern != NULL) {
        driver_terminal_printString("too few arguments, expected: ");
        driver_terminal_printlnString(nextFixedArgumentPattern->displayName);
        return 0;
    }
    segmentCount = repeatedArgumentsWritten;
    repeatedSegmentsConsumed = 0;
    return 1;
}

int shell_processRepeatedArgument(void *storage) {
    if (repeatedSegmentsConsumed == segmentCount) {
        return -1;
    } else {
        int result = parseValue(segments[repeatedSegmentsConsumed], commandPattern->repeatedArguments, storage);
        repeatedSegmentsConsumed++;
        return result;
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
		if (string_equals(commandName, tryCommandPattern->name)) {
			commandPattern = tryCommandPattern;
			break;
		}
	}
	if (commandPattern == NULL) {
		driver_terminal_printString("unknown command: ");
		driver_terminal_printlnString(commandName);
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
	driver_terminal_printString(commandPattern->name);

	// options
	if (commandPattern->options != NULL && commandPattern->options->name != NULL) {
	    driver_terminal_printString(" [options] ");
	} else {
	    driver_terminal_printChar(' ');
	}

	// fixed arguments
	if (commandPattern->fixedArguments != NULL) {
        for (const shell_ValuePattern *argumentPattern = commandPattern->fixedArguments;
                argumentPattern->displayName != NULL;
                argumentPattern++) {
            driver_terminal_printString(" <");
            driver_terminal_printString(argumentPattern->displayName);
            if (argumentPattern->type != shell_ValueType_string) { // don't say "string" when we don't know better
                driver_terminal_printChar(':');
                driver_terminal_printString(shell_valueTypeNames[argumentPattern->type]);
            }
            driver_terminal_printChar('>');
        }
	}

	// repeated arguments
	if (commandPattern->repeatedArguments != NULL) {
		const shell_ValuePattern *argumentPattern = commandPattern->repeatedArguments;
		driver_terminal_printString(" <");
		driver_terminal_printString(argumentPattern->displayName);
        if (argumentPattern->type != shell_ValueType_string) { // don't say "string" when we don't know better
            driver_terminal_printChar(':');
            driver_terminal_printString(shell_valueTypeNames[argumentPattern->type]);
        }
        driver_terminal_printString(" ...>");
	}

}

shell_OptionPattern *shell_findShortOption(const shell_CommandPattern *commandPattern, char name) {
    if (commandPattern->options == NULL) {
        return NULL;
    }
    for (shell_OptionPattern *option = commandPattern->options; option->name != NULL; option++) {
        if (option->name[0] == name && option->name[1] == 0) {
            return option;
        }
    }
    return NULL;
}

shell_OptionPattern *shell_findOption(const shell_CommandPattern *commandPattern, const char *name) {
    if (commandPattern->options == NULL) {
        return NULL;
    }
    for (shell_OptionPattern *option = commandPattern->options; option->name != NULL; option++) {
        if (string_equals(name, option->name)) {
            return option;
        }
    }
    return NULL;
}
