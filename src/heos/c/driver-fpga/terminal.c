
#include <divrem.h>
#include <draw.h>
#include "../driver/terminal.h"

static int x, y;

void driver_terminal_initialize() {
    x = y = 0;
}

static void advance() {
    x += 8;
    if (x == 512) { // my display cuts off the right part of the screen, so 512 instead of 640
        x = 0;
        y += 16;
    }
}

void driver_terminal_printString(const char *s) {
    while (1) {
        char c = *s;
        if (c == 0) {
            return;
        }
        drawCharacter(x, y, c);
        advance();
        s++;
    }
}

void driver_terminal_printChar(char c) {
    drawCharacter(x, y, c);
    advance();
}

void driver_terminal_printInt(int i) {
    if (i < 0) {
        driver_terminal_printChar('-');
        i = -i;
    }
    driver_terminal_printUnsignedInt(i);
}

void driver_terminal_printUnsignedInt(unsigned int i) {

    // special case: since we suppress leading zeroes, actual zero would be invisible without this
    if (i == 0) {
        driver_terminal_printChar('0');
        return;
    }

    // start with the first digit (could be optimized)
    int significance = 1000000000;

    // print digits
    int started = 0;
    while (significance > 0) {
        int digit = udiv(i, significance);
        if (started || digit != 0) {
            driver_terminal_printChar((char)('0' + digit));
            started = 1;
        }
        i -= digit * significance;
        significance = udiv(significance, 10);
    }

}

void driver_terminal_printHexInt(int i) {
    if (i < 0) {
        driver_terminal_printChar('-');
        i = -i;
    }
    driver_terminal_printUnsignedHexInt(i);
}

void driver_terminal_printUnsignedHexInt(unsigned int i) {
    int shiftAmount = 28;
    while (shiftAmount >= 0) {
        int digit = i >> shiftAmount;
        driver_terminal_printChar(digit < 10 ? ('0' + digit) : ('a' + digit - 10));
        i = i & ((1 << shiftAmount) - 1);
        shiftAmount -= 4;
    };
}

void driver_terminal_println() {
    x = 0;
    y += 16;
}

void driver_terminal_printlnString(const char *s) {
    driver_terminal_printString(s);
    driver_terminal_println();
}

void driver_terminal_printlnChar(char c) {
    driver_terminal_printChar(c);
    driver_terminal_println();
}

void driver_terminal_printlnInt(int i) {
    driver_terminal_printInt(i);
    driver_terminal_println();
}

void driver_terminal_printlnUnsignedInt(unsigned int i) {
    driver_terminal_printUnsignedInt(i);
    driver_terminal_println();
}

void driver_terminal_printlnHexInt(int i) {
    driver_terminal_printHexInt(i);
    driver_terminal_println();
}

void driver_terminal_printlnUnsignedHexInt(unsigned int i) {
    driver_terminal_printUnsignedHexInt(i);
    driver_terminal_println();
}
