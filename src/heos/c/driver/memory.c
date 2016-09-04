
#include "memory.h"

static void writeHelper(int address, int data, int bits) {
	int bitMask = (1 << bits) - 1;
	int msbValue = (1 << (bits - 1));
	unsigned int unsignedData = data & bitMask;
	int signedData = unsignedData;
	if (signedData >= msbValue) {
		signedData = signedData - msbValue - msbValue;
	}
	driver_console_formatLine("writing %d-bit value %d / %d / 0x%x to 0x%x", bits, unsignedData, signedData, unsignedData, address);
}

void driver_memory_write8(int address, int data) {
	writeHelper(address, data, 8);
}

void driver_memory_write16(int address, int data) {
	writeHelper(address, data, 16);
}

void driver_memory_write32(int address, int data) {
	writeHelper(address, data, 32);
}

static int readHelper(int address, int bits) {
	driver_console_formatLine("reading %d-bit value from 0x%x", bits, address);
	return 0;
}

int driver_memory_read8(int address) {
	return readHelper(address, 8);
}

int driver_memory_read16(int address) {
	return readHelper(address, 16);
}

int driver_memory_read32(int address) {
	return readHelper(address, 32);
}
