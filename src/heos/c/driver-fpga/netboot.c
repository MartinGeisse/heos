
#include "netboot.h"
#include "../driver/terminal.h"
#include "cpu.h"

// --------------------------------------------------------------------------------------------------------------------
// management interface
// --------------------------------------------------------------------------------------------------------------------

static int managementMdcCycle(int outputBit) {
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
static void managementSend(unsigned int pattern, int bits) {
    while (bits > 0) {
        managementMdcCycle(pattern >> 31);
        pattern = pattern << 1;
        bits--;
    }
}

static void managementWriteRegister(int registerIndex, unsigned short value) {
    managementSend(-1, 32);
    managementSend(0x5f83ffff | (registerIndex << 18), 16); // phy address is 31
    managementSend(((unsigned int)value) << 16, 16);
}

static void managementInitialize(void) {

    // first, make sure any previously started cycle has been finished (reset may happen *during* a cycle)
    managementSend(-1, 32);
    managementSend(-1, 32);
    managementSend(-1, 32);

    // disable PHY address decoding for subsequent write operations (actually should not matter since we're using the
    // right address, but this makes sure a wrong register write doesn't lock us out of the PHY until power down)
    managementWriteRegister(17, 8);

    // configure registers as needed
    managementWriteRegister(0, 0x1000);
    managementWriteRegister(4, 0x01e1);
    managementWriteRegister(18, 0x00ff);

}

// --------------------------------------------------------------------------------------------------------------------
// receiving
// --------------------------------------------------------------------------------------------------------------------

static unsigned int crc(unsigned char *buffer, unsigned int length) {
   unsigned int crc = 0xffffffff;
   for (unsigned int i = 0; i < length; i++) {
      unsigned int byte = buffer[i];
      crc = crc ^ byte;
      for (int j = 0; j < 8; j++) {
         unsigned int mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xedb88320 & mask);
      }
   }
   return ~crc;
}

typedef struct {
    unsigned int length;
    unsigned char *data;
} ReceivedPacket;

static ReceivedPacket receivePacket(void) {
    volatile unsigned int *interface = (volatile unsigned int *)0x08000000;
    while (1) {

        // wait for packet
        while (interface[1] == 0);
        unsigned int packetLength = interface[2];

        // ignore short packets -- must contain at least preamble, source/destination address and ethertype
        if (packetLength < 22) {
            interface[1] = 0;
            continue;
        }

        // detect and skip preamble
        if (interface[1024] != 0x55555555 || interface[1025] != 0xd5555555) {
            interface[1] = 0;
            continue;
        }
        unsigned char *packet = ((unsigned char *)(interface + 1026));
        packetLength -= 8;

        // check ignored ethertypes
        unsigned short etherType = (packet[12] << 8) + packet[13];
        if (etherType == 0x88e1 || etherType == 0x8912) { // FritzBox mesh detection
            interface[1] = 0;
            continue;
        }

        // CRC validation
        unsigned int computedCrc = crc(packet, packetLength - 4);
        unsigned int specifiedCrc = packet[packetLength - 4] + (packet[packetLength - 3] << 8)
            + (packet[packetLength - 2] << 16) + (packet[packetLength - 1] << 24);
        if (computedCrc != specifiedCrc) {
            interface[1] = 0;
            continue;
        }

        // packet successfully received
        ReceivedPacket result;
        result.length = packetLength;
        result.data = packet;
        return result;

    }
}

static void dismissReceivedPacket(void) {
    volatile unsigned int *interface = (volatile unsigned int *)0x08000000;
    interface[1] = 0;
}

// --------------------------------------------------------------------------------------------------------------------
// netboot logic
// --------------------------------------------------------------------------------------------------------------------

static void printHexDigit(int digit) {
    driver_terminal_printChar(digit < 10 ? (digit + '0') : (digit - 10 + 'a'));
}

void netboot(void) {
    managementInitialize();
    driver_terminal_printlnString("--- netboot ---");
    while (1) {

        ReceivedPacket packet = receivePacket();

        // print length
        driver_terminal_printString("received packet; length = ");
        driver_terminal_printlnUnsignedHexInt(packet.length);

        // print first 128 bytes
        if (packet.length > 256) {
            packet.length = 256;
        }
        for (unsigned int i = 0; i < packet.length; i++) {
            unsigned char byte = packet.data[i];
            printHexDigit(byte >> 4);
            printHexDigit(byte & 15);
            driver_terminal_printString("  ");
        }
        driver_terminal_println();

        // finished
        dismissReceivedPacket();
        driver_terminal_printlnString("--------------------------------------------------------");

    }
}
