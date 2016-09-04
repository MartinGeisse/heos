
#ifndef HDR__DRIVER_MEMORY_H__
#define HDR__DRIVER_MEMORY_H__

void driver_memory_write8(int address, int data);
void driver_memory_write16(int address, int data);
void driver_memory_write32(int address, int data);
int driver_memory_read8(int address);
int driver_memory_read16(int address);
int driver_memory_read32(int address);

#endif
