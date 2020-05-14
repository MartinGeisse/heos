
#ifndef HDR__DRIVER_CONSOLE_H__
#define HDR__DRIVER_CONSOLE_H__

void driver_console_readLine(char *destinationBuffer, int bufferSize);

void driver_console_print(const char *contentBuffer);
void driver_console_println(const char *contentBuffer);
void driver_console_format(const char *formatBuffer, ...);
void driver_console_formatln(const char *formatBuffer, ...);

#endif
