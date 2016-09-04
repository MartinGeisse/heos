
#include "driver/console.h"
#include "mainLoop/mainLoop.h"

int main() {
    driver_console_printLine("");
    driver_console_printLine("****************");
    driver_console_printLine("* HEOS started *");
    driver_console_printLine("****************");
    driver_console_printLine("");
    mainLoop_loop();
	return 0;
}
