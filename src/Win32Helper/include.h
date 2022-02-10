#ifndef __WIN32_HELPER__
#define __WIN32_HELPER__
#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

// 检查表达式是否为真
#define CHECK(call) ;\
    if (!(call)) {\
        SetLastError(-1);\
        Perror(#call);\
    }\
;
// 检查Win32系统调用
#define SYSCALL(call) ;\
    SetLastError(0);\
    call;\
    Perror(#call);\
;
// 检查Win32系统调用，同时将返回值赋值给某变量
#define SYSCALL2(call, recv) ;\
    SetLastError(0);\
    recv = call;\
    Perror(#call);\
;
#define Perror(x) _Perror(x, __FILE__, __LINE__)

void win32_init();
void init_locale(const char *locale);
long _Perror(char *op, const char *file, int line);

// ANSI转义序列
#define GREEN "\x1B[32m"
#define RED "\x1B[91m"
#define RESET "\x1B[0m"
void enable_virtual_terminal_processing();

void alloc_console();

void balloon_notification(HWND hwnd, UINT id, const wchar_t *message);

wchar_t *utf8to16(const char *str);
 
char *utf16to8(const wchar_t *str);

#ifdef __cplusplus
}
#endif
#endif // __WIN32_HELPER__