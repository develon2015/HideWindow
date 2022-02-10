#include "include.h"

wchar_t*
utf8to16(const char* str) {
    if (str == NULL) return L"(null)";
    // 计算缓冲区需要的大小, 如果函数成功, 则返回 UTF-8 字符数量, 所以无法确定具体字节数
    size_t cBuf = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (cBuf == 0) return L"(null)";
    wchar_t* buf = (wchar_t*)malloc(cBuf * 4);
    if (cBuf != MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, cBuf)) return L"(null)";
    return buf;
}

char*
utf16to8(const wchar_t* str) {
    if (str == NULL) return "(null)";
    // 计算缓冲区需要的大小, 如果函数成功, 则返回具体字节数, 所以 cBuf 至少是 1 (UTF-8以0x00结尾)
    size_t cBuf = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (cBuf < 1) return "(null)";
    char* buf = (char*)malloc(cBuf); // 分配缓冲区
    if (cBuf != WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, cBuf, NULL, NULL)) return "(null)";
    return buf;
}