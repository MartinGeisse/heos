
#ifndef __DRIVER_TERMINAL_H__
#define __DRIVER_TERMINAL_H__

void driver_terminal_initialize(void);
void driver_terminal_printString(const char *s);
void driver_terminal_printChar(char c);
void driver_terminal_printInt(int i);
void driver_terminal_printUnsignedInt(unsigned int i);
void driver_terminal_printHexInt(int i);
void driver_terminal_printUnsignedHexInt(unsigned int i);
void driver_terminal_println(void);
void driver_terminal_printlnString(const char *s);
void driver_terminal_printlnChar(char c);
void driver_terminal_printlnInt(int i);
void driver_terminal_printlnUnsignedInt(unsigned int i);
void driver_terminal_printlnHexInt(int i);
void driver_terminal_printlnUnsignedHexInt(unsigned int i);

void driver_terminal_readLine(char *buffer, int bufferSize);

#endif
