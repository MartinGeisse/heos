
#include <stdio.h>

unsigned char packet[] = {
    0x03,
    0, 0, 0, 0
};
#define payloadLength 1
#define packetLength 5

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

void generate(void) {
    printf("generate:\n");
    /*
    unsigned int crc = 0xffffffff;
    for (unsigned int i = 0; i < payloadLength; i++) {
        unsigned char byte = packet[i];
        for (int bit = 0; bit < 8; bit++) {
            int msb = crc & 0x80000000;
            crc = (crc << 1) | (byte & 1);
            if (msb) {
                crc ^= 0x4C11DB7;
            }
            byte >>= 1;
        }
    }
//    int crcReversed = 0;
//    for (int i = 0; i < 32; i++) {
//        crcReversed = (crcReversed << 1) | (crc & 1);
//        crc = (crc >> 1);
//    }
    // crcReversed = ~crcReversed;
    crc = ~crc;
    printf("%08x\n", crc);
    */
    printf("%08x\n", crc(packet, payloadLength));
}

void validate(void) {
//        packet[packetLength - 1] = ~packet[packetLength - 1];
//        packet[packetLength - 2] = ~packet[packetLength - 2];
//        packet[packetLength - 3] = ~packet[packetLength - 3];
//        packet[packetLength - 4] = ~packet[packetLength - 4];
//    unsigned int crc = 0xffffffff;
//    for (unsigned int i = 0; i < packetLength; i++) {
//        unsigned char byte = packet[i];
//        for (int bit = 0; bit < 8; bit++) {
//            if ((crc ^ byte) & 1) {
//                crc >>= 1;
//                crc ^= 0xedb88320U;
//            } else {
//                crc >>= 1;
//            }
//        }
//    }
}

int main(void) {
    generate();
    validate();
    return 0;
}
