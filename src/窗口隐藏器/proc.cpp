#include "include.h"
#include <thread>
#include <CommCtrl.h>

static HMENU mainmenu;

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CLOSE:
    {
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }
    case WM_SIZE:
    {
        MoveWindow(text, 0, 0, LOWORD(lParam), HIWORD(lParam), true);
        if (wParam == SIZE_MINIMIZED) {
            ShowWindow(hWnd, SW_HIDE);
        }
        return 0;
    }
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case 200:
        {
            enum_window();
            break;
        }
        case 101:
        {
            system("start https://github.com/develon2015/HideWindow");
            break;
        }
        case 102:
        {
            SYSCALL(Shell_NotifyIconW(NIM_DELETE, pnid));
            ExitProcess(0);
            break;
        }
        default:
        {
            switch (HIWORD(wParam))
            {
            case LBN_DBLCLK:
            {
                LRESULT index = SendMessage(text, LB_GETCURSEL, 0, 0);
                if (hwnds[index]) {
                    if (index >= hwnds_length - hided_hwnds_length) {
                        ShowWindow(hwnds[index], SW_SHOW);
                        for (int i = 0; i < hided_hwnds_length; i++) {
                            if (hwnds[index] == hided_hwnds[i]) {
                                for (int j = i; j < hided_hwnds_length; j++) {
                                    hided_hwnds[j] = hided_hwnds[j + 1];
                                }
                                hided_hwnds_length--;
                                break;
                            }
                        }
                        enum_window();
                    } else {
                        ShowWindow(hwnds[index], SW_HIDE);
                        hided_hwnds[hided_hwnds_length++] = hwnds[index];
                        enum_window();
                    }
                }
                break;
            }
            }
        }
        }
        break;
    }
    case WM_USER + 1:
    {
        switch (lParam)
        {
        case WM_LBUTTONDBLCLK:
        {
            enum_window();
            break;
        }
        case WM_RBUTTONDOWN:
        {
            if (mainmenu == NULL)
            {
                mainmenu = CreatePopupMenu();
                AppendMenuW(mainmenu, MF_STRING, 200, L"主程序");
                AppendMenuW(mainmenu, MF_SEPARATOR, (UINT)0, NULL);

                HMENU submenu = CreatePopupMenu();
                AppendMenuW(submenu, MF_STRING, 101, L"主页");
                AppendMenuW(mainmenu, MF_POPUP, (UINT_PTR)submenu, L"关于");

                AppendMenuW(mainmenu, MF_SEPARATOR, (UINT)0, NULL);
                AppendMenuW(mainmenu, MF_STRING, 102, L"退出");
            }

            POINT p;
            GetCursorPos(&p);
            SetForegroundWindow(hWnd), TrackPopupMenu(mainmenu, TPM_LEFTBUTTON, p.x, p.y, 0, hwnd, 0);

            break;
        }
        }
        break;
    }
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

// https://stackoverflow.com/questions/31648180/c-changing-hwnd-window-procedure-in-runtime
LRESULT CALLBACK mySubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
        {
            if (wParam == VK_RETURN) {
                int buf[128] = {0};
                LRESULT count = SendMessage(text, LB_GETSELITEMS, (WPARAM)sizeof buf, (LPARAM)&buf);
                for (int i = 0; i < count; i++) {
                    int index = buf[i];
                    if (hwnds[index]) {
                        if (index >= hwnds_length - hided_hwnds_length) {
                            ShowWindow(hwnds[index], SW_SHOW);
                            for (int i = 0; i < hided_hwnds_length; i++) {
                                if (hwnds[index] == hided_hwnds[i]) {
                                    for (int j = i; j < hided_hwnds_length; j++) {
                                        hided_hwnds[j] = hided_hwnds[j + 1];
                                    }
                                    hided_hwnds_length--;
                                    break;
                                }
                            }
                            // enum_window();
                        } else {
                            ShowWindow(hwnds[index], SW_HIDE);
                            hided_hwnds[hided_hwnds_length++] = hwnds[index];
                            // enum_window();
                        }
                    }
                }
                enum_window();
            }
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}