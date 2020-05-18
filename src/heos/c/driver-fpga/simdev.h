
#ifndef __DRIVER_SIMDEV_H__
#define __DRIVER_SIMDEV_H__

int simdevIsSimulation(void);
void simdevMessage(char *message);
void simdevShowInt(char *label, int value);
void simdevFillWordsShowInt(void *pointer, int value, int wordCount);
void simdevSelectDisplayPlane(int plane);

void simdevGlFlipScreen(void);
void simdevGlClearScreen(int color);

#endif
