
#include <stdlib.h>
#include "../driver/terminal.h"
#include "commands.h"

// TODO: output info
//
// 	driver_terminal_formatln("reading %d-bit value from 0x%x", bits, address);
//
//	int bitMask = (1 << bits) - 1;
//	int msbValue = (1 << (bits - 1));
//	unsigned int unsignedData = data & bitMask;
//	int signedData = unsignedData;
//	if (signedData >= msbValue) {
//		signedData = signedData - msbValue - msbValue;
//	}
// 	driver_terminal_formatln("writing %d-bit value %d / %d / 0x%x to 0x%x", bits, unsignedData, signedData, unsignedData, address);
//

static void _cmd_help() {
    if (!shell_processOptionsAndFixedArguments(NULL)) {
        return;
    }
	driver_terminal_printlnString("List of commands:");
	for (int i=0; i<shell_commandPatternCount; i++) {
		const shell_CommandPattern *commandPattern = shell_commandPatterns + i;
		driver_terminal_printString("    ");
		shell_printSynopsis(commandPattern);
		driver_terminal_println();
	}
}

/*
static void _cmd_write8(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	driver_memory_write8(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void _cmd_write16(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	driver_memory_write16(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void _cmd_write32(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	driver_memory_write32(arguments[0].properties[0].asInt, arguments[1].properties[0].asInt);
}

static void readHelper(int data, int bits) {
	int bitMask = (1 << bits) - 1;
	int msbValue = (1 << (bits - 1));
	unsigned int unsignedData = data & bitMask;
	int signedData = unsignedData;
	if (signedData >= msbValue) {
		signedData = signedData - msbValue - msbValue;
	}
	driver_terminal_formatln("read %d-bit value %d / %d / 0x%x", bits, unsignedData, signedData, unsignedData);
}

static void _cmd_read8(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(driver_memory_read8(arguments[0].properties[0].asInt), 8);
}

static void _cmd_read16(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(driver_memory_read16(arguments[0].properties[0].asInt), 16);
}

static void _cmd_read32(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	readHelper(driver_memory_read32(arguments[0].properties[0].asInt), 32);
}

static void _cmd_dump8(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	int address = arguments[0].properties[0].asInt;
	for (int row = 0; row < 16; row++) {
		driver_terminal_format("%08x:", address);
		for (int column = 0; column < 16; column++) {
			driver_terminal_format(" %02x", driver_memory_read8(address) & 0xff);
			address++;
		}
		driver_terminal_println("");
	}
}

static void _cmd_dump16(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	int address = arguments[0].properties[0].asInt;
	for (int row = 0; row < 16; row++) {
		driver_terminal_format("%08x:", address);
		for (int column = 0; column < 8; column++) {
			driver_terminal_format(" %04x", driver_memory_read16(address) & 0xffff);
			address+=2;
		}
		driver_terminal_println("");
	}
}

static void _cmd_dump32(int __attribute__((unused)) argumentCount, const shell_ParsedArgument *arguments) {
	int address = arguments[0].properties[0].asInt;
	for (int row = 0; row < 16; row++) {
		driver_terminal_format("%08x:", address);
		for (int column = 0; column < 4; column++) {
			driver_terminal_format(" %08x", ((unsigned int)driver_memory_read32(address)) & 0xffffff);
			address+=4;
		}
		driver_terminal_println("");
	}
}
*/

const shell_CommandPattern shell_commandPatterns[] = {
	{
	    .name = "help",
	    .options = NULL,
	    .fixedArguments = NULL,
	    .repeatedArguments = NULL,
	    .callback = _cmd_help
    },
/*
	{.name = "write8", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_write8},
	{.name = "write16", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_write16},
	{.name = "write32", .fixedArgumentCount = 2, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
		{.name = "data", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_write32},
	{.name = "read8", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_read8},
	{.name = "read16", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_read16},
	{.name = "read32", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "address", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_read32},
	{.name = "dump8", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "baseAddress", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_dump8},
	{.name = "dump16", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "baseAddress", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_dump16},
	{.name = "dump32", .fixedArgumentCount = 1, .fixedArguments = &((shell_ArgumentPattern[]) {
		{.name = "baseAddress", .type = &shell_intArgumentType},
	}), .repeatedArguments = NULL, .callback = _cmd_dump32},
*/
};

const int shell_commandPatternCount = sizeof(shell_commandPatterns) / sizeof(shell_CommandPattern);
