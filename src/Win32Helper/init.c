#include "include.h"

void win32_init() {
#ifndef NDEBUG
    // 执行字符集是UTF-8
    init_locale("zh_CN.UTF-8");
    alloc_console();
#else
    // 执行字符集是GBK
    init_locale("zh_CN.BGK");
    // alloc_console();
#endif
    enable_virtual_terminal_processing();
}