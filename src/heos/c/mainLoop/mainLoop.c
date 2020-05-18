
#include "mainLoop.h"
#include "../driver/terminal.h"
#include "../shell/shell.h"

static int mainLoopAborted = 0;
static char commandLine[256];

void mainLoop_loop(void) {
    while (!mainLoopAborted) {
        driver_terminal_readLine(commandLine, 256);
        shell_executeCommandLine(commandLine);
    }
}

void mainLoop_setAborted(int aborted) {
    mainLoopAborted = aborted;
}
