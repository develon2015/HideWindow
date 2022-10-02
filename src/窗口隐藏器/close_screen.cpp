#include "include.h"

static BOOL hasRegisted = FALSE;
static HHOOK hhook = NULL;
static HWND black = NULL;

static auto password = "JAVA";
static auto le = strlen(password);
static int progress = 0;

static int unlock = 0;

static LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
	// Do the wParam and lParam parameters contain information about a keyboard message.
	if (nCode == HC_ACTION)
	{
		// Messsage data is ready for pickup
		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
		// if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
		{
            if (p->vkCode == VK_F12 && (p->flags & 0b00100000)) { // 长按Alt+F12解锁
                unlock++;
            } else {
                unlock = 0;
            }
            printf("%c\n", password[progress]);
            if (p->vkCode == password[progress]) {
                progress++;
            } else {
                progress = 0;
            }
            auto ok = progress >= le;
            if (unlock >= 30 || ok) {
                unlock = 0;
                UnhookWindowsHookEx(hhook);
                DestroyWindow(black);
                return TRUE;
            }
		}
	}
    return TRUE;
	// hook procedure must pass the message *Always*
	// return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
        {
            ShowWindow(hWnd, SW_MAXIMIZE);
            hhook = SetWindowsHookExW(
                WH_KEYBOARD_LL,        // low-level keyboard input events
                HookProcedure,         // pointer to the hook procedure
                GetModuleHandle(NULL), // A handle to the DLL containing the hook procedure
                NULL                   // desktop apps, if this parameter is zero
            );
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            GetClientRect(hWnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            EndPaint(hWnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

static HWND registe() {
    const wchar_t *cname = L"black_window";
    if (!hasRegisted) {
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof wc;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = cname;
        wc.hIcon = LoadIconA(NULL, IDI_SHIELD);
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        SYSCALL(RegisterClassExW(&wc));
        hasRegisted = TRUE;
    }

    DWORD exstyle = 0;
    const wchar_t *name = L"sleeping...";
    DWORD style = WS_POPUP;
    int x = 0,
        y = 0,
        w = GetSystemMetrics(SM_CXSCREEN),
        h = GetSystemMetrics(SM_CYSCREEN);
    HWND parent = NULL;
    HMENU menu = NULL;
    SYSCALL2(GetModuleHandleW(NULL), HINSTANCE inst);
    CHECK(inst != NULL);
    LPVOID param = 0;

    SYSCALL2(CreateWindowExW(exstyle, cname, name, style, x, y, w, h, parent, menu, inst, param), HWND hwnd);
    CHECK(hwnd != NULL);

    return hwnd;
}

void close_screen() {
    black = registe();
}