
#include "lan.h"
#include "../driver/terminal.h"
#include "cpu.h"
#include "keyboard.h"

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
//static int receive(int bits) {
//    int result = 0;
//    while (bits > 0) {
//        result = (result << 1) | mdcCycle(1);
//        bits--;
//    }
//    return result;
//}

static void sendCycleHeader(int write, int registerIndex) {
    send(-1, 32);
    send((write ? 0x5f83ffff : 0x6f83ffff) | (registerIndex << 18), 16); // phy address is 31
}

//static int readManagementRegister(int registerIndex) {
//    sendCycleHeader(0, registerIndex);
//    return receive(16);
//}

static void writeManagementRegister(int registerIndex, unsigned short value) {
    sendCycleHeader(1, registerIndex);
    send(((unsigned int)value) << 16, 16);
}

void lan_initialize(void) {

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

}

static void printHexDigit(int digit) {
    driver_terminal_printChar(digit < 10 ? (digit + '0') : (digit - 10 + 'a'));
}

void lanTest(void) {
    driver_terminal_printlnString("--- begin LAN test ---");

    volatile unsigned int *interface = (volatile unsigned int *)0x08000000;
    while (1) {

        // wait for packet
        while (interface[1] == 0);
        unsigned int packetLength = interface[2];

        // detect and skip preamble
        if (interface[1024] != 0x55555555 || interface[1025] != 0xd5555555) {
            interface[1] = 0;
            continue;
        }
        unsigned char *packet = ((unsigned char *)(interface + 1026));

        // ignore short packets -- must contain at least source/destination address and ethertype
        if (packetLength < 14) {
            interface[1] = 0;
            continue;
        }

        // check ignored ethertypes
        unsigned short etherType = (packet[12] << 8) + packet[13];
        if (etherType == 0x88e1 || etherType == 0x8912) { // FritzBox mesh detection
            interface[1] = 0;
            continue;
        }

        // print length
        driver_terminal_printString("received packet; length = ");
        driver_terminal_printlnUnsignedHexInt(packetLength);

        // print first 128 bytes
        if (packetLength > 128) {
            packetLength = 128;
        }
        for (unsigned int i = 0; i < packetLength; i++) {
            unsigned char byte = packet[i];
            printHexDigit(byte >> 4);
            printHexDigit(byte & 15);
            driver_terminal_printString("  ");
        }
        driver_terminal_println();

        // wait for keypress
        while (!KEY_STATE(0x5a));
        while (KEY_STATE(0x5a));

        // acknowledge received packet
        interface[1] = 0;

    }

    driver_terminal_printlnString("--- LAN test finished ---");
}
