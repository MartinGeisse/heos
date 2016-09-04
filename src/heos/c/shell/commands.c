
#include <stdlib.h>
#include "../console/console.h"
#include "commands.h"

static void _cmd_help(int __attribute__((unused)) argumentCount, const shell_ParsedArgument * __attribute__((unused)) arguments) {
	console_printLine("List of commands:");
	for (int i=0; i<shell_commandPatternCount; i++) {
		const shell_CommandPattern *commandPattern = shell_commandPatterns + i;
		console_print("    ");
		shell_printSynopsis(commandPattern);
		console_printLine("");
	}
}

static void _cmd_write8(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	write8(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void _cmd_write16(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	write16(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void _cmd_write32(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	write32(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void readHelper(int data, int bits) {
	int bitMask = (1 << bits) - 1;
	int msbValue = (1 << (bits - 1));
	unsigned int unsignedData = data & bitMask;
	int signedData = unsignedData;
	if (signedData >= msbValue) {
		signedData = signedData - msbValue - msbValue;
	}
	console_formatLine("read %d-bit value %d / %d / 0x%x", bits, unsignedData, signedData, unsignedData);
}

static void _cmd_read8(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(read8(arguments[0].properties[0].asInt), 8);
}

static void _cmd_read16(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(read16(arguments[0].properties[0].asInt), 16);
}

static void _cmd_read32(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(read32(arguments[0].properties[0].asInt), 32);
}

const shell_CommandPattern shell_commandPatterns[] = {
	{.name = "help", .fixedArgumentCount = 0, .fixedArguments = NULL, .repeatedArgument = NULL, .callback = _cmd_help},
	{.name = "write8", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_write8},
	{.name = "write16", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_write16},
	{.name = "write32", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_write32},
	{.name = "read8", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_read8},
	{.name = "read16", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_read16},
	{.name = "read32", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArgument = NULL, .callback = _cmd_read32},
};

const int shell_commandPatternCount = sizeof(shell_commandPatterns) / sizeof(shell_CommandPattern);
