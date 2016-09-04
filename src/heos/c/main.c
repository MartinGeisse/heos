
#include "console/console.h"
#include "mainLoop/mainLoop.h"

int main() {
    console_printLine("");
    console_printLine("****************");
    console_printLine("* HEOS started *");
    console_printLine("****************");
    console_printLine("");
    mainLoop_loop();
	return 0;
}
