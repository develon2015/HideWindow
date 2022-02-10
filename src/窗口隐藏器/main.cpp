#include "include.h"
#include <commctrl.h>

HWND hwnd = NULL;
HWND text = NULL;
HFONT hFont = CreateFontW(
    20, 0, //高度20, 宽取0表示由系统选择最佳值
    0, 0,    //文本倾斜，与字体倾斜都为0
    FW_NORMAL,
    0, 0, 0,                //非斜体，无下划线，无中划线
    GB2312_CHARSET,         //字符集
    OUT_DEFAULT_PRECIS,     //输出精度
    CLIP_DEFAULT_PRECIS,    //裁剪精度
    CLEARTYPE_NATURAL_QUALITY, //输出质量
    DEFAULT_PITCH | FF_DONTCARE,
    L"微軟正黑體" //字体名称
);

HWND main_window() {
    const wchar_t *cname = L"main";
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof wc;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = cname;
	wc.hIcon = LoadIconA(NULL, IDI_SHIELD);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    SYSCALL(RegisterClassExW(&wc));

    DWORD exstyle = 0;
    const wchar_t *name = L"窗口列表";
    DWORD style = WS_OVERLAPPEDWINDOW;
    int x = 600,
        y = 200,
        w = 400,
        h = 800;
    HWND parent = NULL;
    HMENU menu = NULL;
    SYSCALL2(GetModuleHandleW(NULL), HINSTANCE inst);
    CHECK(inst != NULL);
    LPVOID param = 0;

    SYSCALL2(CreateWindowExW(exstyle, cname, name, style, x, y, w, h, parent, menu, inst, param), HWND hwnd);
    CHECK(hwnd != NULL);
    style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL | LBS_MULTIPLESEL | LBS_NOTIFY;
    SYSCALL2(CreateWindowExW(0, WC_LISTBOXW, L"list", style, 0, 0, 0, 0, hwnd, NULL, inst, NULL), text);
    SendMessage(text, WM_SETFONT, (WPARAM)hFont, TRUE); // 设置控件字体
    SetWindowSubclass(text, mySubClassProc, 0, 0); // 设置子类
    return hwnd;
}

NOTIFYICONDATAW *pnid = NULL;
static NOTIFYICONDATAW nid = {0};

void system_tray() {
    if (pnid == NULL) {
        pnid = &nid;
        nid.cbSize = sizeof nid;
        nid.uID = -8888;
        nid.uVersion = NOTIFYICON_VERSION;
        // nid.uVersion = NOTIFYICON_VERSION_4; // 版本4似乎无法显示悬浮提示
        nid.hWnd = hwnd; // 必须设置所属窗口，否则鼠标移到任务栏图标上时会闪退

        nid.uFlags |= NIF_ICON;
        nid.hIcon = LoadIconA(NULL, IDI_SHIELD);

        nid.uFlags |= NIF_TIP;
        lstrcpyW(nid.szTip, L"窗口隐藏器");

        nid.uFlags |= NIF_INFO;
        nid.dwInfoFlags = NIIF_ICON_MASK;
        lstrcpyW(nid.szInfoTitle, L"气球通知");
        lstrcpyW(nid.szInfo, L"这是一条气球通知");

        nid.uFlags |= NIF_MESSAGE;
        nid.uCallbackMessage = WM_USER + 1;

        nid.uFlags ^= NIF_INFO; // 使用异或关闭气球通知
    }
    SYSCALL(Shell_NotifyIconW(NIM_ADD, &nid));
    SYSCALL(Shell_NotifyIconW(NIM_SETVERSION, &nid));
}

BOOL EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t classname[256] = {0};
    GetClassNameW(hwnd, classname, sizeof classname);
        wchar_t title[256] = {0};
        GetWindowTextW(hwnd, title, sizeof title);
        if (IsWindowVisible(hwnd)) {
            printf("%ls --> %ls\n", classname, title);
        }
    return true;
}

BOOL EnumWindowsProc2(HWND hwnd, LPARAM lParam) {
    wchar_t classname[256] = {0};
    GetClassNameW(hwnd, classname, sizeof classname);
    if (lstrcmpW(classname, L"ConsoleWindowClass") == 0) {
        ShowWindow(hwnd, SW_SHOW);
    }
    return true;
}

int wWinMain(HINSTANCE inst, HINSTANCE prev, wchar_t *cmd, int mode) {
	win32_init();
    printf("%ls\n", RED L"程序运行中..." RESET);

    CHECK((hwnd = main_window(), hwnd != NULL));
    // ShowWindow(hwnd, SW_SHOW);

    system_tray();

    MSG msg = {0};
    while (1) {
        CHECK(GetMessageW(&msg, NULL, 0, 0));
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
