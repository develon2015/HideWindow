#include "include.h"
#include <stdio.h>

#define TAG "[%s: %d] \t"

long _Perror(char *op, const char *file, int line) {
    wchar_t *bufMsg = NULL;
    long code = GetLastError();
    if (code == 0) {
        printf(GREEN TAG "%s: OK\n" RESET, file, line, op);
        return code;
    }

    BOOL ok = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, // 可以使用LoadLibraryEx()加载netmsg.dll模块获取其句柄
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&bufMsg,
        0,
        NULL);

    if (!ok) {
        printf(RED TAG "%s: ERROR(%d)\n" RESET, file, line, op, code);
        return -1;
    }
    printf(RED TAG, file, line);
    wprintf(L"%hs: %Ts", op, bufMsg);
    printf(RESET);

    LocalFree(bufMsg);
    SetLastError(0);
    return code;
}