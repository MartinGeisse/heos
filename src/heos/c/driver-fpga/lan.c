
#include "lan.h"
#include "../driver/terminal.h"
#include "cpu.h"

static int mdcCycle(int outputBit) {
    outputBit = outputBit & 1;
    volatile int *clock = (int*)0x07000000;
    volatile int *interface = (int*)0x08000000;
    // each tick of the hardware clock is 16 clock cycles, i.e. 160ns. We want to wait 200ns, but we might "almost"
    // miss one tick, so we wait three ticks to be safe. Then, missing one means we waited at least 320ns.
    int stop = 3 + *clock;
    while (*clock < stop);
    *interface = outputBit;
    int result = *interface;
    stop = 3 + *clock;
    while (*clock < stop);
    *interface = outputBit | 2;
    return result;
}

// first bit is bit 31 of the pattern, next is bit 30, and so on
static void send(unsigned int pattern, int bits) {
    while (bits > 0) {
        mdcCycle(pattern >> 31);
        pattern = pattern << 1;
        bits--;
    }
}

// will store the result in the (bits) lowest bits
static int receive(int bits) {
    int result = 0;
    while (bits > 0) {
        result = (result << 1) | mdcCycle(1);
        bits--;
    }
    return result;
}

static void sendCycleHeader(int write, int registerIndex) {
    send(-1, 32);
    send((write ? 0x5f83ffff : 0x6f83ffff) | (registerIndex << 18), 16); // phy address is 31
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

    // disable PHY address decoding for subsequent write operations (actually should not matter since we're using the
    // right address, but this makes sure a wrong register write doesn't lock us out of the PHY until power down)
    writeManagementRegister(17, 8);

    // configure registers as needed
    writeManagementRegister(0, 0x1000);
    writeManagementRegister(4, 0x01e1);
    writeManagementRegister(18, 0x00ff);

    // test read
    while (1) {
        driver_terminal_initialize();
        for (int i = 0; i < 32; i += 4) {
            driver_terminal_printString("registers ");
            driver_terminal_printInt(i);
            driver_terminal_printString("-");
            driver_terminal_printInt(i + 3);
            driver_terminal_printString(": ");

            driver_terminal_printUnsignedHexInt(readManagementRegister(i));
            driver_terminal_printString(", ");
            driver_terminal_printUnsignedHexInt(readManagementRegister(i + 1));
            driver_terminal_printString(", ");
            driver_terminal_printUnsignedHexInt(readManagementRegister(i + 2));
            driver_terminal_printString(", ");
            driver_terminal_printUnsignedHexInt(readManagementRegister(i + 3));

            driver_terminal_println();
        }
        delay(500);
    }

    driver_terminal_printlnString("--- LAN test finished ---");
}
