
#ifndef HDR__CONSOLE_CONSOLE_H__
#define HDR__CONSOLE_CONSOLE_H__

void console_readLine(char *destinationBuffer, int bufferSize);
void console_print(const char *contentBuffer);
void console_format(const char *formatBuffer, ...);
void console_printLine(const char *contentBuffer);
void console_formatLine(const char *formatBuffer, ...);

#endif
