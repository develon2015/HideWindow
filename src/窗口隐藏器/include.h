#ifndef __INCLUDE_H__
#define __INCLUDE_H__
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "Win32Helper/include.h"

extern HWND hwnd;
extern HWND text;
extern NOTIFYICONDATAW *pnid;
LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
LRESULT CALLBACK mySubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
void enum_window();

extern HWND hwnds[1024];
extern int hwnds_length;
extern HWND hided_hwnds[1024];
extern int hided_hwnds_length;

#endif // __INCLUDE_H__