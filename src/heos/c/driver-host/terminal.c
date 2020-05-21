
#include <termios.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../driver/terminal.h"


void driver_terminal_initialize() {
    struct termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag &= ~ICANON & ~ECHO;
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

void driver_terminal_printString(const char *s) {
    printf("%s", s);
}

void driver_terminal_printChar(char c) {
    putchar(c);
}

void driver_terminal_printInt(int i) {
    printf("%d", i);
}

void driver_terminal_printUnsignedInt(unsigned int i) {
    printf("%u", i);
}

void driver_terminal_printHexInt(int i) {
    if (i < 0) {
        putchar('-');
        i = -i;
    }
    if (i < 0) {
        puts("0x80000000");
        return;
    }
    printf("%x", (unsigned int)i);
}

void driver_terminal_printUnsignedHexInt(unsigned int i) {
    printf("%x", i);
}

void driver_terminal_println() {
    putchar('\n');
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


void driver_terminal_readLine(char *buffer, int bufferSize) {
/*
    while (1) {
        printf("> ");
        if (fgets(buffer, bufferSize, stdin) == NULL) {
            printf("ERROR reading from stdin\n");
            exit(1);
        }
        int length = strlen(buffer);
        if (length + 1 == bufferSize) {
            printf("input too long\n");
            continue;
        }
        if (buffer[length - 1] == '\n') {
            buffer[length - 1] = 0;
        }
        return;
    }
*/

    if (bufferSize < 1) {
        printf("input buffer too small\n");
        exit(1);
    }
    int length = 0;
    buffer[0] = 0;
    printf("> \033[s");
    while (1) {
        printf("\033[u%s\033[K", buffer);
        int c = getchar();
        if (c == 27) {
            c = getchar();
            if (c == '[') {
                c = getchar();
                switch (c) {

//                    case 'A':
//                        break;
//
//                    case 'B':
//                        break;
//
//                    case 'C':
//                        break;
//
//                    case 'D':
//                        break;

                    default:
                        printf("*** escape: %d\n", c);

                }
            }
        } else if (c == '\n') {
            printf("\n");
            return;
        } else if (c < 32) {
            // ignored
        } else if (c == 127) {
            if (length > 0) {
                length--;
                buffer[length] = 0;
            }
        } else {
            if (length < bufferSize - 1) {
                buffer[length] = c;
                length++;
                buffer[length] = 0;
            }
        }
    }
}
