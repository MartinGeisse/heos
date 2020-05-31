
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

const unsigned short HEOS_CONTROL_ETHERTYPE = 0x8abc;
// const unsigned short HEOS_RESET_ETHERTYPE = 0x8cba;

void die(const char *message) {
    printf("ERROR: %s\n", message);
    exit(1);
}

void dieErrno(void) {
    die(strerror(errno));
}

int createSocket(void) {
    int socketFd = socket(AF_PACKET, SOCK_DGRAM, htons(HEOS_CONTROL_ETHERTYPE));
    if (socketFd == -1) {
        dieErrno();
    }
    return socketFd;
}

int findInterfaceIndex(int socket, const char *interfaceName) {
    struct ifreq ifr;
    strcpy(ifr.ifr_name, interfaceName);
    if (ioctl(socket, SIOCGIFINDEX, &ifr) == -1) {
        dieErrno();
    }
    return ifr.ifr_ifindex;
}

struct sockaddr_ll getBroadcastAddress(int interfaceIndex) {
    const unsigned char broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct sockaddr_ll address = {0};
    address.sll_family = AF_PACKET;
    address.sll_ifindex = interfaceIndex;
    address.sll_halen = ETHER_ADDR_LEN;
    address.sll_protocol = htons(HEOS_CONTROL_ETHERTYPE);
    memcpy(address.sll_addr, broadcastAddress, ETHER_ADDR_LEN);
    return address;
}

void sendPacket(int socket, struct sockaddr_ll destinationAddress, void *buffer, unsigned int length) {
    if (sendto(socket, buffer, length, 0, (struct sockaddr*)&destinationAddress, sizeof(destinationAddress)) == -1) {
        dieErrno();
    }
}

int main(void) {

    // parameters
    const char *interfaceName = "enp1s0";
//    unsigned char payload[] = {
//        0x11, 0x22, 0x33, 0x44
//    };
    const char *imageFilename = "out-fpga/program.bin";

    // open input image file
    FILE *inputFile = fopen(imageFilename, "rb");
    if (inputFile == NULL) {
        dieErrno();
    }
    fseek(inputFile, 0L, SEEK_END);
    int fileSize = ftell(inputFile);
    fseek(inputFile, 0L, SEEK_SET);
    printf("file size: %d\n", fileSize);

    // prepare network
    int socket = createSocket();
    printf("socket: %d\n", socket);
    int interfaceIndex = findInterfaceIndex(socket, interfaceName);
    printf("interface index: %d\n", interfaceIndex);
    struct sockaddr_ll broadcastAddress = getBroadcastAddress(interfaceIndex);

    // send the header packet
    unsigned char headerPacket[] = {

        // packet type
        0x00, 0x00, 0x00, 0x00,

        // file size
        (unsigned char)(fileSize),
        (unsigned char)(fileSize >> 8),
        (unsigned char)(fileSize >> 16),
        (unsigned char)(fileSize >> 24),

    };
    sendPacket(socket, broadcastAddress, headerPacket, sizeof(headerPacket));

    // send file contents; send it multiple times in case a packet gets overlooked
    for (int i = 0; i < 5; i++) {
        printf("run %d\n", i);
        fseek(inputFile, 0L, SEEK_SET);
        for (int position = 0; position < fileSize; position += 1024) {
            if ((position & 16383) == 0) {
                printf("  position: %d\n", position);
            }
            unsigned char bodyPacket[1032] = {

                // packet type
                0x01, 0x00, 0x00, 0x00,

                // position
                (unsigned char)(position),
                (unsigned char)(position >> 8),
                (unsigned char)(position >> 16),
                (unsigned char)(position >> 24),

            };
            fread(bodyPacket + 8, 1, 1024, inputFile);
            sendPacket(socket, broadcastAddress, bodyPacket, sizeof(bodyPacket));

        }
    }
    return 0;

}
