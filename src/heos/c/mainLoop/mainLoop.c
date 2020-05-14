
#include "mainLoop.h"
#include "../driver/console.h"
#include "../shell/shell.h"

static int mainLoopAborted = 0;
static char commandLine[256];

void mainLoop_loop() {
    while (!mainLoopAborted) {
        driver_console_readLine(commandLine, 256);
        shell_executeCommandLine(commandLine);
    }
}

void mainLoop_setAborted(int aborted) {
    mainLoopAborted = aborted;
}
