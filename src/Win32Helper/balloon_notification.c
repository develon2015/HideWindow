#include "include.h"

void balloon_notification(HWND hwnd, UINT id, const wchar_t *message) {
    NOTIFYICONDATAW nid = {0};
    nid.uID = id;
    nid.cbSize = sizeof nid;
    nid.hWnd = hwnd; // 必须要设置hWnd

    nid.uFlags |= NIF_ICON;
    nid.hIcon = LoadIconA(NULL, IDI_SHIELD);

    // nid.uFlags |= NIF_MESSAGE;
    // nid.uCallbackMessage = WM_USER + 2;

    nid.uFlags |= NIF_INFO;
    nid.dwInfoFlags = NIIF_ICON_MASK; // 必须要设置hIcon
    lstrcpyW(nid.szInfoTitle, L"通知");
    lstrcpyW(nid.szInfo, message);
    SYSCALL(Shell_NotifyIconW(NIM_ADD, &nid));

    Sleep(80); // 延迟隐藏以显示进程名，似乎会导致无法接收消息
    // 隐藏图标
    nid.uFlags |= NIF_STATE;
    nid.dwState = NIS_HIDDEN;
    nid.dwStateMask = NIS_HIDDEN;
    SYSCALL(Shell_NotifyIconW(NIM_MODIFY, &nid));
}