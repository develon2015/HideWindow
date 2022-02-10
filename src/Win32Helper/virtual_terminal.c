#include "include.h"

void enable_virtual_terminal_processing() {
    HANDLE handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(handle_stdout, &mode);
    SYSCALL(SetConsoleMode(handle_stdout, ENABLE_VIRTUAL_TERMINAL_PROCESSING | mode));
}