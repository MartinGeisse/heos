
#ifndef __DRIVER_KEYBOARD_H__
#define __DRIVER_KEYBOARD_H__

extern volatile unsigned char keyStateTable[32];
#define KEY_STATE(scancode) ((keyStateTable[(scancode) >> 3] & (1 << ((scancode) & 7))) != 0)

unsigned char fetchKeyboardScanCode(void);

#endif
