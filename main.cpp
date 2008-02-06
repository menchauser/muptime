/**
 * (c) Muhamed Karanashev, 2008
 */
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include "resource.h"

#define ADDTIMEPART(time, str)        if (wcslen(str) <= 1) wcscat(time, _T("0\0")); wcscat(time, str);
#define UCALLBACKMESSAGE 0x1001

static TCHAR szWindowClass[] = _T("uptimeAppClass");
static TCHAR szTitle[] = _T("muptime");
static BOOL isMinimized = FALSE;
static UINT POS_X = 0;
static UINT POS_Y = 48;
static wchar_t iniFile[MAX_PATH];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
wchar_t* getUptimeStr(wchar_t*);
void setFont(LOGFONT*);
void copyTextToClipboard(HWND, wchar_t*);
void readSettingsFromFile(LPCTSTR fileName);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nCmdShow) {
    WNDCLASSEX wnd;
    wnd.cbSize = sizeof(WNDCLASSEX);
    wnd.style  = CS_HREDRAW | CS_VREDRAW;
    wnd.lpfnWndProc = WndProc;
    wnd.cbClsExtra  = 0;
    wnd.cbWndExtra  = 0;
    wnd.hInstance   = hInst;
    wnd.hIcon  = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    wnd.lpszMenuName = NULL;
    wnd.lpszClassName = szWindowClass;
    wnd.hIconSm = LoadIcon(wnd.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    if (!RegisterClassEx(&wnd)) {
        MessageBoxW(NULL,
            L"Ошибка вызова RegisterClassEx!",
            L"muptime",
            NULL);
        return 1;
    }

    GetCurrentDirectory(MAX_PATH, iniFile);
    wcscat_s(iniFile, _T("\\muptime.ini"));
    POS_X = GetPrivateProfileInt(_T("Position"), _T("X"), POS_X, iniFile);
    POS_Y = GetPrivateProfileInt(_T("Position"), _T("Y"), POS_Y, iniFile);
    HWND hWnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP | WS_DLGFRAME, //WS_DLGFRAME
        POS_X, POS_Y,
        96, 25,
        NULL,
        NULL,
        hInst,
        NULL);
    if (!hWnd) {
        MessageBoxW(NULL,
            L"Ошибка вызова CreateWindow",
            L"muptime",
            NULL);
        return 1;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    SetWindowPos(hWnd, HWND_TOPMOST, POS_X, POS_Y, 0, 0, SWP_NOSIZE);
    MSG msg;
    {
        NOTIFYICONDATA ntdata;
        ntdata.cbSize = sizeof(NOTIFYICONDATA);
        ntdata.hWnd = hWnd;
        ntdata.uID = 12;//идентификатор пиктограммы на панели задач
        ntdata.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
        ntdata.uCallbackMessage = UCALLBACKMESSAGE;
        ntdata.hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
        wcscpy_s(ntdata.szTip, _T("muptime"));
        Shell_NotifyIcon(NIM_ADD, &ntdata);
    }
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam) {
    const UINT_PTR IDT_TIMER1 = 1;
    wchar_t *time = new wchar_t[16];
    LOGFONT lg;
    HFONT fnt;
    PAINTSTRUCT ps;
    HDC hdc;
    switch(message) {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            setFont(&lg);
            fnt = CreateFontIndirect(&lg);
            SelectObject(hdc, fnt);
            getUptimeStr(time);
            TextOutW(hdc, 3, 3, time, wcslen(time));
            EndPaint(hWnd, &ps);
            DeleteObject(fnt);
            break;
        case WM_CREATE:
            SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
            break;
        case WM_DESTROY:
            {
                LPRECT rect = new tagRECT;
                GetWindowRect(hWnd, rect);
                wchar_t x_pos[4];
                wchar_t y_pos[4];
                _ltow(rect->left, x_pos, 10);
                _ltow(rect->top, y_pos, 10);
                WritePrivateProfileString(_T("Position"), _T("X"), x_pos, iniFile);
                WritePrivateProfileString(_T("Position"), _T("Y"), y_pos, iniFile);
                delete rect;
            }
            KillTimer(hWnd, IDT_TIMER1);
            PostQuitMessage(0);
            break;
        case WM_TIMER:
            hdc = GetDC(hWnd);
            setFont(&lg);
            fnt = CreateFontIndirect(&lg);
            SelectObject(hdc, fnt);
            getUptimeStr(time);
            TextOutW(hdc, 3, 3, time, wcslen(time));
            DeleteObject(fnt);
            ReleaseDC(hWnd, hdc);
            break;
        case WM_CHAR:
            if (MapVirtualKey(wparam, MAPVK_VK_TO_CHAR) == 27)
                SendMessage(hWnd, WM_DESTROY, wparam, lparam);
            if (wparam == 'c' || wparam == 'C') {
                getUptimeStr(time);
                copyTextToClipboard(hWnd, time);
            }
            if (wparam = 'q' || wparam == 'Q')
                SendMessage(hWnd, WM_SIZE, SIZE_MINIMIZED, NULL);
            break;
        case WM_LBUTTONDOWN:
            SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
            break;
        case WM_RBUTTONUP:
            getUptimeStr(time);
            copyTextToClipboard(hWnd, time);
            break;
        case WM_SIZE:
            if (wparam == SIZE_MINIMIZED) {
                NOTIFYICONDATA ntdata;
                ntdata.cbSize = sizeof(NOTIFYICONDATA);
                ntdata.hWnd = hWnd;
                ntdata.uID = 12;//идентификатор пиктограммы на панели задач
                ntdata.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
                ntdata.uCallbackMessage = UCALLBACKMESSAGE;
                ntdata.hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
                wcscpy(ntdata.szTip, _T("muptime"));
                Shell_NotifyIcon(NIM_ADD, &ntdata);
                ShowWindow(hWnd, SW_HIDE);
            }
            break;
        case UCALLBACKMESSAGE:
            if (wparam == 12) {
                switch (lparam) {
                    case WM_LBUTTONDBLCLK:
                        if (isMinimized) {
                            ShowWindow(hWnd, SW_SHOWNORMAL);
                            isMinimized = FALSE;
                        }
                        else {
                            NOTIFYICONDATA ntdata;
                            ntdata.cbSize = sizeof(NOTIFYICONDATA);
                            ntdata.hWnd = hWnd;
                            ntdata.uID = 12;//идентификатор пиктограммы на панели задач
                            ntdata.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
                            ntdata.uCallbackMessage = UCALLBACKMESSAGE;
                            ntdata.hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
                            wcscpy_s(ntdata.szTip, _T("muptime"));
                            Shell_NotifyIcon(NIM_ADD, &ntdata);
                            ShowWindow(hWnd, SW_HIDE);
                            isMinimized = TRUE;
                        }
                        break;
                    case WM_RBUTTONUP:
                        getUptimeStr(time);
                        copyTextToClipboard(hWnd, time);
                        break;
                }
            }
            break;
        case WM_QUIT:
            {
                NOTIFYICONDATA ntdata;
                ntdata.cbSize = sizeof(NOTIFYICONDATA);
                ntdata.hWnd = hWnd;
                ntdata.uID = 12;//идентификатор пиктограммы на панели задач
                ntdata.uFlags = NIF_MESSAGE;
                ntdata.uCallbackMessage = UCALLBACKMESSAGE;
                Shell_NotifyIcon(NIM_DELETE, &ntdata);
            }
            
            break;
        default:
            return DefWindowProc(hWnd, message, wparam, lparam);
            break;
    }
    delete[] time;
    return 0;
}

wchar_t* getUptimeStr(wchar_t* dst) {
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

void setFont(LOGFONT *lg) {
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
    wcscpy_s(lg->lfFaceName, strName);
}

void copyTextToClipboard(HWND hWnd, wchar_t* src) {
    //копируем src в буфер окна hWnd
    if(OpenClipboard(hWnd)){
        EmptyClipboard();
        SetClipboardData(CF_TEXT, (HANDLE)src);
        CloseClipboard();
    }
}

void readSettingsFromFile(LPCTSTR fileName) {

}