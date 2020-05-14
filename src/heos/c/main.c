
#include "driver/console.h"
#include "mainLoop/mainLoop.h"

int main(void) {

    driver_console_format("");
    driver_console_format("", 0);
    driver_console_format("", 0, 0);
    driver_console_format("", 0, 0, 0);
    driver_console_format("", 0, 0, 0, 0);
    driver_console_format("", 0, 0, 0, 0, 0);
    driver_console_format("", 0, 0, 0, 0, 0, 0);
    driver_console_format("", 0, 0, 0, 0, 0, 0, 0);


    driver_console_println("");
    driver_console_println("****************");
    driver_console_println("* HEOS started *");
    driver_console_println("****************");
    driver_console_println("");
    mainLoop_loop();
	return 0;
}
