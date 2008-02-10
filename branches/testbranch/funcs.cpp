#define _UNICODE
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "main.h"
#include <memory>
using namespace std;
// TODO (Muhamed#1#): установить везде поддержку auto_ptr

/**
 * функция форматирует строку с аптаймом
 * @param dst строка, в которую записывается аптайм
 * @return Указатель на результирующую строку
 */
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
    auto_ptr<wchar_t> time(new wchar_t[16]);
    swprintf(time.get(), _T("%02ld:%02ld:%02ld:%02ld"), days, hours, minutes, seconds);
    wcsncpy(dst, time.get(), 16);
    return dst;
}

/**
 * устанавливает параметры нашего шрифты
 * @param lg указатель на структуру tagFONT, в которую пишем параметры
 */
void setFontProperties(LOGFONTW *lg) {
    //создаем шрифт
    const wchar_t strName[] = _T("SomeUptimeFont");
    lg->lfHeight = 14;
    lg->lfWidth  = 6;
    lg->lfEscapement = 0;
    lg->lfOrientation = 0;
    lg->lfWeight = FW_MEDIUM;
    lg->lfItalic = 0;
    lg->lfUnderline = 0;
    lg->lfStrikeOut = 0;
    lg->lfCharSet = ANSI_CHARSET;
    lg->lfOutPrecision = OUT_STROKE_PRECIS;
    lg->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lg->lfQuality = ANTIALIASED_QUALITY;
    lg->lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    wcscpy(lg->lfFaceName, strName);
}

/**
 * копирует юникодный текст в буфер обмена
 * @param hWnd дескриптор окна, в чей буфер обмена копируем
 * @param src юникодная строка, которую копируем в буфер обмена
 */
void copyTextToClipboard(HWND hWnd, wchar_t* src) {
    //копируем src в буфер окна hWnd
    if(OpenClipboard(hWnd)){
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, (HANDLE)src);
        CloseClipboard();
    }
}

/**
 * выводит аптайм в устройство с заданным контекстом
 * @param hdc контекст устройства вывода
 * @param uptime строка с аптаймом
 * @return результат функции DrawTextW
 */
int drawUptime(HDC hdc, wchar_t *uptime, UINT width, UINT height) {
    LPRECT rect = new tagRECT;
    rect->left = 1;
    rect->top = 1;
    rect->right = width - 8;
    rect->bottom = height - 8;
    //SetBkMode(hdc, TRANSPARENT);
    return DrawTextW(hdc, uptime, wcslen(uptime), rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
    delete rect;
}
