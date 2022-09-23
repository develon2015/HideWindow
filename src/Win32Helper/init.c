#include "include.h"

void win32_init() {
#ifndef NDEBUG
    alloc_console();
#endif
    // 执行字符集是GBK
    // init_locale("zh_CN.UTF-8");
    enable_virtual_terminal_processing();
}