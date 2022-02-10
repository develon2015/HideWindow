#include "include.h"

static wchar_t buf[1024] = {0};

HWND hwnds[1024] = {0};
int hwnds_length = 0;
HWND hided_hwnds[1024] = {0};
int hided_hwnds_length = 0;

BOOL CALLBACK enum_window_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return true;
    GetWindowTextW(hwnd, buf, sizeof buf);
    if (lstrcmpW(buf, L"窗口列表") == 0) return true;
    if (lstrcmpW(buf, L"") == 0) {
        GetClassNameW(hwnd, buf, sizeof buf);
    }
    SendMessageW(text, LB_ADDSTRING, 0, (LPARAM)buf);
    hwnds[hwnds_length++] = hwnd;
    return true;
}

void enum_window() {
    SendMessageW(text, LB_RESETCONTENT, 0, (LPARAM)buf);
    hwnds_length = 0;
    EnumWindows(enum_window_callback, NULL);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetForegroundWindow(hwnd);
    // 已隐藏
    if (hided_hwnds_length > 0) {
        SendMessageW(text, LB_ADDSTRING, 0, (LPARAM)L"=============已隐藏==============");
        hwnds[hwnds_length++] = NULL;
        for (int i = 0; i < hided_hwnds_length; i++) {
            GetWindowTextW(hided_hwnds[i], buf, sizeof buf);
            if (lstrcmpW(buf, L"") == 0) {
                GetClassNameW(hwnd, buf, sizeof buf);
            }
            SendMessageW(text, LB_ADDSTRING, 0, (LPARAM)buf);
            hwnds[hwnds_length++] = hided_hwnds[i];
        }
    }
}