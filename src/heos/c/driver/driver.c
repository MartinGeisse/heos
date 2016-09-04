
#include "driver.h"

static void writeHelper(int address, int data, int bits) {
	int bitMask = (1 << bits) - 1;
	int msbValue = (1 << (bits - 1));
	unsigned int unsignedData = data & bitMask;
	int signedData = unsignedData;
	if (signedData >= msbValue) {
		signedData = signedData - msbValue - msbValue;
	}
	console_formatLine("writing %d-bit value %d / %d / 0x%x to 0x%x", bits, unsignedData, signedData, unsignedData, address);
}

void write8(int address, int data) {
	writeHelper(address, data, 8);
}

void write16(int address, int data) {
	writeHelper(address, data, 16);
}

void write32(int address, int data) {
	writeHelper(address, data, 32);
}

static int readHelper(int address, int bits) {
	console_formatLine("reading %d-bit value from 0x%x", bits, address);
	return 0;
}

int read8(int address) {
	readHelper(address, 8);
}

int read16(int address) {
	readHelper(address, 16);
}

int read32(int address) {
	readHelper(address, 32);
}
