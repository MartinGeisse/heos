
#include "lan.h"
#include "../driver/terminal.h"

static int mdcWait(int fallingEdgeOutputValue) {
    int *clock = (int*)0x07000000;
    int *interface = (int*)0x08000000;
    // each tick of the hardware clock is 16 clock cycles, i.e. 160ns. We want to wait 200ns, but we might "almost"
    // miss one tick, so we wait three ticks to be safe. Then, missing one means we waited at least 320ns.
    int stop = 3 + *clock;
    while (*clock < stop);
    *interface = fallingEdgeOutputValue;
    int result = *interface;
    stop = 3 + *clock;
    while (*clock < stop);
    return result;
}

// first bit is bit 31 of the pattern, next is bit 30, and so on
static void send(unsigned int pattern, int bits) {
    int *interface = (int*)0x08000000;
    while (bits > 0) {
        unsigned int shifted = pattern >> 31;
        mdcWait(shifted);
        *interface = shifted | 2;
        pattern = pattern << 1;
        bits--;
    }
}

// will store the result in the (bits) lowest bits
static int receive(int bits) {
    int *interface = (int*)0x08000000;
    int result = 0;
    while (bits > 0) {
        result = (result << 1) | mdcWait(1);
        *interface = 3;
        bits--;
    }
    return result;
}

static void sendCycleHeader(int write, int registerIndex) {
    mdcWait(1);
    send(-1, 32);
    int value = (write ? 0x5003ffff : 0x6003ffff) | (registerIndex << 18);
    send(value, 16);
}

static int readManagementRegister(int registerIndex) {
    sendCycleHeader(0, registerIndex);
    return receive(16);
}

static void writeManagementRegister(int registerIndex, unsigned short value) {
    sendCycleHeader(1, registerIndex);
    send(((unsigned int)value) << 16, 16);
}

void lanTest(void) {
    driver_terminal_printlnString("--- begin LAN test ---");

    // first, make sure any previously started cycle has been finished (reset may happen *during* a cycle)
    send(-1, 32);
    send(-1, 32);
    send(-1, 32);

    // test read
    for (int i = 0; i < 1; i++) { // TODO 32
        int value = readManagementRegister(i);
        driver_terminal_printString("register ");
        driver_terminal_printInt(i);
        driver_terminal_printString(": ");
        driver_terminal_printUnsignedHexInt(value);
        driver_terminal_println();
    }

    driver_terminal_printlnString("--- LAN test finished ---");
}
