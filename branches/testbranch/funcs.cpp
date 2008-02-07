#define _UNICODE
#include <windows.h>
#include <tchar.h>
#define ADDTIMEPART(time, str)        if (wcslen(str) <= 1) wcscat(time, _T("0\0")); wcscat(time, str);

wchar_t* getUptimeStr(wchar_t* dst) {
    //форматируем/заполняем строку аптаймом в формате dd:hh:mm:ss
    long milliseconds = GetTickCount();
    long days = milliseconds / 86400000;
    milliseconds -= days * 86400000;
    long hours = milliseconds / 3600000;
    milliseconds -= hours * 3600000;
    long minutes = milliseconds / 60000;
    milliseconds -= minutes * 60000;
    long seconds = milliseconds / 1000;

    wchar_t* time = new wchar_t[16];
    memset(time, 0, 16);

    wchar_t* sDays = new wchar_t[3];
    sDays = _ltow(days, sDays, 10);
    ADDTIMEPART(time, sDays);
    wcscat(time, _T(":\0"));
    delete[] sDays;
    wchar_t* sHours = new wchar_t[3];
    sHours = _ltow(hours, sHours, 10);
    ADDTIMEPART(time, sHours);
    wcscat(time, _T(":\0"));
    delete[] sHours;
    wchar_t* sMinutes = new wchar_t[3];
    sMinutes = _ltow(minutes, sMinutes, 10);
    ADDTIMEPART(time, sMinutes);
    wcscat(time, _T(":\0"));
    delete[] sMinutes;
    wchar_t* sSeconds = new wchar_t[3];
    sSeconds = _ltow(seconds, sSeconds, 10);
    ADDTIMEPART(time, sSeconds);
    delete[] sSeconds;
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
