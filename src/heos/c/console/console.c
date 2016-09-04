
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "console.h"

void console_readLine(char *destinationBuffer, int bufferSize) {
    fgets(destinationBuffer, bufferSize, stdin);
    destinationBuffer[strlen(destinationBuffer) - 1] = 0;
}

void console_print(const char *contentBuffer) {
	printf("%s", contentBuffer);
}

void console_format(const char *formatBuffer, ...) {
    va_list args;
    va_start(args, formatBuffer);
    vprintf(formatBuffer, args);
    va_end(args);
}

void console_printLine(const char *contentBuffer) {
    puts(contentBuffer);
}

void console_formatLine(const char *formatBuffer, ...) {
    va_list args;
    va_start(args, formatBuffer);
    vprintf(formatBuffer, args);
    va_end(args);
    putc('\n', stdout);
}
