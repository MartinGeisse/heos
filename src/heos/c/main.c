
#include "driver/terminal.h"
#include "mainLoop/mainLoop.h"
#include "driver-fpga/keyboard.h"
#include "driver-fpga/lan.h"

int main(void) {
    driver_terminal_initialize();
    driver_terminal_println();
    driver_terminal_printlnString("****************");
    driver_terminal_printlnString("* HEOS started *");
    driver_terminal_printlnString("****************");
    driver_terminal_println();

    lanTest();

    // mainLop_loop();

    while (1) {
        int scanCode = fetchKeyboardScanCode();
        if (scanCode != 0) {
            driver_terminal_printString("in: ");
            driver_terminal_printlnHexInt(scanCode);
        }
    }

	return 0;
}
