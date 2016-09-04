
#ifndef HDR__DRIVER_H__
#define HDR__DRIVER_H__

void write8(int address, int data);
void write16(int address, int data);
void write32(int address, int data);
int read8(int address);
int read16(int address);
int read32(int address);

#endif
