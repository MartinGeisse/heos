
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "console.h"

void driver_console_readLine(char *destinationBuffer, int bufferSize) {
    fgets(destinationBuffer, bufferSize, stdin);
    destinationBuffer[strlen(destinationBuffer) - 1] = 0;
}

void driver_console_print(const char *contentBuffer) {
	printf("%s", contentBuffer);
}

void driver_console_format(const char *formatBuffer, ...) {
    va_list args;
    va_start(args, formatBuffer);
    vprintf(formatBuffer, args);
    va_end(args);
}

void driver_console_println(const char *contentBuffer) {
    puts(contentBuffer);
}

void driver_console_formatln(const char *formatBuffer, ...) {
    va_list args;
    va_start(args, formatBuffer);
    vprintf(formatBuffer, args);
    va_end(args);
    putc('\n', stdout);
}
