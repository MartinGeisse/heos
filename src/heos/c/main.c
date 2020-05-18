
#include "driver/terminal.h"
#include "mainLoop/mainLoop.h"

int main(void) {
    driver_terminal_println();
    driver_terminal_printlnString("****************");
    driver_terminal_printlnString("* HEOS started *");
    driver_terminal_printlnString("****************");
    driver_terminal_println();
    mainLoop_loop();
	return 0;
}
