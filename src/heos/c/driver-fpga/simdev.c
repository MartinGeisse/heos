
#include "simdev.h"

#define OPENGL_BASE_ADDRESS = 0x10000;

static volatile int *simulationDevice = (volatile int *)0x40000000;
static volatile int *openglDevice = (volatile int *)0x40040000;

int simdevIsSimulation() {
    return *simulationDevice;
}

void simdevMessage(char *message) {
    simulationDevice[1] = 0;
}

void simdevShowInt(char *label, int value) {
    simulationDevice[1] = 1;
}

void simdevFillWordsShowInt(void *pointer, int value, int wordCount) {
    simulationDevice[2] = 0;
}

void simdevSelectDisplayPlane(int plane) {
    simulationDevice[3] = plane;
}

void simdevGlFlipScreen() {
    openglDevice[0] = 0;
}

void simdevGlClearScreen(int color) {
    openglDevice[1] = color;
}
