
#include "lan.h"
#include "../driver/terminal.h"
#include "cpu.h"
#include "keyboard.h"

const unsigned char myMacAddress[6] = {
    0x12, 0x34, 0x56, 0xfe, 0xdc, 0xba
};

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

static unsigned int assembleBeforeSendBuffer[400];

void sendPacket(unsigned char *destinationMacAddress, unsigned short etherType, int length, void *untypedData) {

    // argument validation TODO minimum frame size is 64, max is 1522! (min PAYLOAD size is 1500)
    if (length < 0 || length > 1500) {
        driver_terminal_printString("ERROR: trying to send ethernet frame with invalid length: ");
        driver_terminal_printlnInt(length);
        return;
    }
    if (etherType < 1536) {
        etherType = length;
    }
    unsigned char *data = (unsigned char *)untypedData;

    // assemble the packet
    unsigned char *packet = (unsigned char *)assembleBeforeSendBuffer;
    for (int i = 0; i < 6; i++) {
        packet[i] = destinationMacAddress[i];
        packet[6 + i] = myMacAddress[i];
    }
    packet[12] = etherType >> 8;
    packet[13] = etherType & 0xff;
    for (int i = 0; i < length; i++) {
        packet[14 + i] = data[i];
    }
    unsigned int crcValue = crc(data, length);
    packet[14 + length] = (crcValue & 0xff);
    packet[15 + length] = (crcValue >> 8) & 0xff;
    packet[16 + length] = (crcValue >> 16) & 0xff;
    packet[17 + length] = (crcValue >> 24);

    // transfer data to the hardware and send it
    volatile unsigned int *interface = (volatile unsigned int *)0x08000000;
    interface[1024] = 0x55555555;
    interface[1025] = 0xd5555555;
    for (int i = 0; i < 400 / 4; i++) {
        interface[1026 + i] = assembleBeforeSendBuffer[i];
    }
    interface[3] = 8 + 18 + length;

}

unsigned char testPayload[] = {
    0x44, 0x88, 0xaa, 0xcc
};
unsigned char testDestinationAddress[] = {
    // 0xe0, 0x28, 0x6d, 0x04, 0xc7, 0xeb
    0x8c, 0xec, 0x4b, 0xb2, 0xde, 0x8d
};

void lanTest(void) {
    driver_terminal_printlnString("--- begin LAN test ---");

    // send test packet
//    if (0) {
//        sendPacket(testDestinationAddress, 0x8abc, 4, testPayload);
//    }

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
        packetLength -= 8;

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
        if (packetLength > 256) {
            packetLength = 256;
        }
        for (unsigned int i = 0; i < packetLength; i++) {
            unsigned char byte = packet[i];
            printHexDigit(byte >> 4);
            printHexDigit(byte & 15);
            driver_terminal_printString("  ");
        }
        driver_terminal_println();

        // CRC validation TODO
        driver_terminal_printString("CRC: ");
        driver_terminal_printlnUnsignedHexInt(crc(packet, packetLength - 4));

        // CRC validation TODO
        {
            unsigned int computedCrc = crc(packet, packetLength - 4);
            unsigned int specifiedCrc = packet[packetLength - 4] + (packet[packetLength - 3] << 8)
                + (packet[packetLength - 2] << 16) + (packet[packetLength - 1] << 24);
            driver_terminal_printlnString((computedCrc == specifiedCrc) ? "CRC ok" : "INVALID CRC!");
        }

        // wait for keypress
        while (!KEY_STATE(0x5a));
        while (KEY_STATE(0x5a));

        // acknowledge received packet
        interface[1] = 0;
        driver_terminal_printlnString("--------------------------------------------------------");

    }

    driver_terminal_printlnString("--- LAN test finished ---");
}
