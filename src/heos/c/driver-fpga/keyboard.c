
#include "keyboard.h"

extern unsigned char scanCodeBuffer[32];

static int readIndex = 0;

unsigned char fetchKeyboardScanCode(void) {
    int result = scanCodeBuffer[readIndex];
    if (result != 0) {
        readIndex = (readIndex + 1) & 31;
    }
    return result;
}
