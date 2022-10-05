#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>
#include <thread>
#include "Win32Helper/include.h"

#define DllExport   __declspec( dllexport )

#undef _tWinMain

#define SHOW_CONSOLE 1 // 是否显示控制台窗口

#define WIDTH GetSystemMetrics(SM_CXSCREEN)
#define HEIGHT 200
#define COLOR 0x000000ff

static HINSTANCE hInst = GetModuleHandle(NULL); // 从控制台获取到应用程序实例句柄
static HWND consoleWnd = GetConsoleWindow(); // 控制台窗口句柄
static HWND mainWnd = NULL;
static HWND txtWnd = NULL;
static HBRUSH brush = CreateSolidBrush(0xffffff); // static控件背景刷新画笔
static wchar_t *data = NULL;

static
void createChildWindow(HWND parent) {
	txtWnd = CreateWindowEx(
		0,
		_T("static"), (_TCHAR*)_T(""),
		WS_CHILD | WS_VISIBLE | // 可见控件
		SS_CENTER | SS_CENTERIMAGE, // 水平、垂直居中
		0, 0,
		WIDTH, HEIGHT,
		parent,
		(HMENU)NULL,
		NULL,
		NULL);
	Perror(_T("创建按钮控件"));
	HFONT hFont;    //字体句柄  
	hFont = CreateFontW(
		36, 0,    //高度20, 宽取0表示由系统选择最佳值  
		0, 0,    //文本倾斜，与字体倾斜都为0  
		FW_NORMAL,
		0, 0, 0,        //非斜体，无下划线，无中划线  
		GB2312_CHARSET,    //字符集  
		OUT_DEFAULT_PRECIS, //输出精度
		CLIP_DEFAULT_PRECIS, //裁剪精度
		NONANTIALIASED_QUALITY, //输出质量
		DEFAULT_PITCH | FF_DONTCARE,
		// L"微軟正黑體"    //字体名称  
		L"楷体"    //字体名称  
	);
	Perror(_T("创建字体"));
	SYSCALL(
		SendMessageW(txtWnd, WM_SETFONT, (WPARAM)hFont, TRUE); // 设置控件字体
	)
	SetWindowTextW(txtWnd, data);
}

static
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CTLCOLORSTATIC: {
		{
			// we're about to draw the static
			// set the text colour in (HDC)lParam
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, COLOR);
			// NOTE: per documentation as pointed out by selbie, GetSolidBrush would leak a GDI handle.
			return (LRESULT)brush;
		}
		break;
	}
	case WM_CREATE: {
		createChildWindow(hwnd);
		break;
	}
	case WM_CLOSE: {
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// HBRUSH brush = CreateSolidBrush(0xffffff);
		FillRect(hdc, &ps.rcPaint, brush);
		//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	// Register the window class.
	const _TCHAR CLASS_NAME[] = _T("Float Window Class");

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS); // 可调用SetCursor

	SYSCALL(
		RegisterClass(&wc);
	)

	int scrWidth  = GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = GetSystemMetrics(SM_CYSCREEN);
	// Create the window.
	mainWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW | // 1. 不在任务栏显示。 2. 不显示在Alt + Tab的切换列表中。 3. 在任务管理器的窗口管理Tab中不显示。
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT |
		0,
		CLASS_NAME,                     // Window class
		_T("主窗口"),                   // Window text
		WS_POPUP |
		0,
		// Size and position
		(scrWidth - WIDTH) / 2, (scrHeight - HEIGHT),
		WIDTH, HEIGHT,
		consoleWnd,       // Parent window
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);
	// Perror(_T("创建窗口"));

	if (mainWnd == NULL) {
		return 0;
	}

	//SetWindowLong(mainWnd, GWL_EXSTYLE, GetWindowLong(mainWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// LWA_COLORKEY 0x00000001 使用crKey作为透明度颜色。
	// 指定在组合分层窗口时要使用的透明度颜色键。窗口以这种颜色绘制的所有像素都将是透明的。
	SetLayeredWindowAttributes(mainWnd, 0xffffff, (BYTE)-1, LWA_COLORKEY);
	// LWA_ALPHA 0x00000002 使用bAlpha确定分层窗口的不透明度。
	// Alpha 值用于描述分层窗口的不透明度。当bAlpha为 0 时，窗口是完全透明的。当bAlpha为 255 时，窗口是不透明的。
	// SetLayeredWindowAttributes(mainWnd, RGB(0, 0, 0), 50, LWA_ALPHA);

	ShowWindow(mainWnd, SW_SHOW);
	UpdateWindow(mainWnd);

	// Run the message loop.
	MSG msg = { 0 };
	while (GetMessage(&msg, mainWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

extern "C"
DllExport
int init_float_window() {
	if (txtWnd) { // inited
		return 0;
	}
	std::thread task([]{
		_tWinMain(hInst, NULL, _T(""), SW_SHOW);
	});
	task.detach();
	return 0;
}

static auto lastTriggerTime = 0;
static BOOL flag = FALSE; // 是否有取消任务
#define TIME 2000 // 消息显示时间

extern "C"
DllExport
void toast(const wchar_t* str) {
	if (!flag) {
		flag = TRUE;
		std::thread task([] {
			while (1) {
				Sleep(200);
				if (clock() - lastTriggerTime > TIME) {
					ShowWindow(mainWnd, SW_HIDE);
					break;
				}
			}
			flag = FALSE;
		});
		task.detach();
	}
	lastTriggerTime = clock();
	if (!txtWnd) { // txtWnd还未准备就绪
		auto len = wcslen(str);
		wchar_t *buf = new wchar_t[len];
		wcscpy(buf, str);
		data = buf;
		return;
	}
	SetWindowTextW(txtWnd, str);
	ShowWindow(mainWnd, SW_SHOW);
}

extern "C"
DllExport
void setPos(int x, int y) {
	SYSCALL(
		SetWindowPos(mainWnd, NULL, x, y, -1, -1, SWP_NOSIZE);
	)
}

extern "C"
DllExport
void setWH(int w, int h) {
	SetWindowPos(mainWnd, NULL, -1, -1, w, h, SWP_NOMOVE);
}

extern "C"
DllExport
void setFont(int size, char *font) {
	const wchar_t * _font = (font == NULL ? L"微軟正黑體" : utf8to16(font));
	HFONT hFont;    //字体句柄  
	hFont = CreateFontW(
		size, 0,    //高度20, 宽取0表示由系统选择最佳值  
		0, 0,    //文本倾斜，与字体倾斜都为0  
		FW_NORMAL,
		0, 0, 0,        //非斜体，无下划线，无中划线  
		GB2312_CHARSET,    //字符集  
		OUT_DEFAULT_PRECIS, //输出精度
		CLIP_DEFAULT_PRECIS, //裁剪精度
		NONANTIALIASED_QUALITY, //输出质量，不抗锯齿
		DEFAULT_PITCH | FF_DONTCARE,
		_font //字体名称
	);
	SendMessageW(txtWnd, WM_SETFONT, (WPARAM)hFont, TRUE); // 设置控件字体
	Perror(_T("设置字体"));
}
