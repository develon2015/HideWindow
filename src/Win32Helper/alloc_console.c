#include "include.h"
#include <stdio.h>

void alloc_console() {
    // 分配控制台，并使用特殊文件名"CONOUT$"重新打开输入流stdout: FILE *
	AllocConsole();
	FILE* file = NULL;
	freopen_s(&file, "CONOUT$", "w+", stdout);
}