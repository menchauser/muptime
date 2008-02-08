#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <windows.h>

wchar_t* getUptimeStr(wchar_t* uptime);
void setFontProperties(LOGFONTW* lf);
void copyTextToClipboard(HWND hWnd, wchar_t* text);
int drawUptime(HDC hdc, wchar_t* uptime, UINT width, UINT height);

#endif // MAIN_H_INCLUDED
