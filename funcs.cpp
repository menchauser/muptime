#define _UNICODE
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

wchar_t *getUptimeStr(wchar_t *dst) {
    //форматируем строку с аптаймом
    long milliseconds = GetTickCount();
    long days = milliseconds / 86400000;
    milliseconds -= days * 86400000;
    long hours = milliseconds / 3600000;
    milliseconds -= hours * 3600000;
    long minutes = milliseconds / 60000;
    milliseconds -= minutes * 60000;
    long seconds = milliseconds / 1000;
    wchar_t* time = new wchar_t[16];
    swprintf(time, _T("%02ld:%02ld:%02ld:%02ld"), days, hours, minutes, seconds);
    wcsncpy(dst, time, 16);
    delete[] time;
    return dst;
}

void setFontProperties(LOGFONTW *lg) {
    //создаем шрифт
    const wchar_t strName[] = _T("SomeUptimeFont");
    lg->lfHeight = 14;
    lg->lfWidth  = 8;
    lg->lfEscapement = 0;
    lg->lfOrientation = 0;
    lg->lfWeight = 120;
    lg->lfItalic = 0;
    lg->lfUnderline = 0;
    lg->lfStrikeOut = 0;
    lg->lfCharSet = ANSI_CHARSET;
    lg->lfOutPrecision = 0;
    lg->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lg->lfQuality = PROOF_QUALITY;
    lg->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    wcscpy(lg->lfFaceName, strName);
}

void copyTextToClipboard(HWND hWnd, wchar_t* src) {
    //копируем src в буфер окна hWnd
    if(OpenClipboard(hWnd)){
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, (HANDLE)src);
        CloseClipboard();
    }
}
