/**
 * (c) Muhamed Karanashev, 2008
 */

#include <windows.h>
#include <tchar.h>
#include "resource.h"

#define ADDTIMEPART(time, str)        if (strlen(str) <= 1) strcat(time, "0\0"); strcat(time, str);
#define UCALLBACKMESSAGE 0x1001

static TCHAR szWindowClass[] = _T("uptimeAppClass");
static TCHAR szTitle[] = _T("muptime");
static BOOL isMinimized = FALSE;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char* getUptimeStr(char*);
void setFont(LOGFONT*);
void copyTextToClipboard(HWND, char*);

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

    HWND hWnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP | WS_DLGFRAME, //WS_DLGFRAME
        0, 48,
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
    SetWindowPos(hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
    MSG msg;
    {
        NOTIFYICONDATA ntdata;
        ntdata.cbSize = sizeof(NOTIFYICONDATA);
        ntdata.hWnd = hWnd;
        ntdata.uID = 12;//идентификатор пиктограммы на панели задач
        ntdata.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
        ntdata.uCallbackMessage = UCALLBACKMESSAGE;
        ntdata.hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
        strcpy(ntdata.szTip, "muptime");
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
    char *time = new char[16];
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
            TextOutA(hdc, 3, 3, time, strlen(time));
            EndPaint(hWnd, &ps);
            DeleteObject(fnt);
            break;
        case WM_CREATE:
            SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
            break;
        case WM_DESTROY:
            KillTimer(hWnd, IDT_TIMER1);
            PostQuitMessage(0);
            break;
        case WM_TIMER:
            hdc = GetDC(hWnd);
            setFont(&lg);
            fnt = CreateFontIndirect(&lg);
            SelectObject(hdc, fnt);
            getUptimeStr(time);
            TextOutA(hdc, 3, 3, time, strlen(time));
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
                strcpy(ntdata.szTip, "muptime");
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
                            strcpy(ntdata.szTip, "muptime");
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

char* getUptimeStr(char* dst) {
    long milliseconds = GetTickCount();
    long days = milliseconds / 86400000;
    milliseconds -= days * 86400000;
    long hours = milliseconds / 3600000;
    milliseconds -= hours * 3600000;
    long minutes = milliseconds / 60000;
    milliseconds -= minutes * 60000;
    long seconds = milliseconds / 1000;
    
    char* time = new char[16];
    memset(time, 0, 16);

    char* sDays = new char[3];
    sDays = ltoa(days, sDays, 10);
    ADDTIMEPART(time, sDays);
    strcat(time, ":\0");
    delete[] sDays;
    char* sHours = new char[3];
    sHours = ltoa(hours, sHours, 10);
    ADDTIMEPART(time, sHours);
    strcat(time, ":\0");
    delete[] sHours;
    char* sMinutes = new char[3];
    sMinutes = ltoa(minutes, sMinutes, 10);
    ADDTIMEPART(time, sMinutes);
    strcat(time, ":\0");
    delete[] sMinutes;
    char* sSeconds = new char[3];
    sSeconds = ltoa(seconds, sSeconds, 10);
    ADDTIMEPART(time, sSeconds);
    delete[] sSeconds;
    strncpy(dst, time, 16);
    delete[] time;
    return dst;
}

void setFont(LOGFONT *lg) {
    const char strName[] = "SomeUptimeFont";
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
    strcpy(lg->lfFaceName, strName);
}

void copyTextToClipboard(HWND hWnd, char* src) {
    //копируем src в буфер окна hWnd
    if(OpenClipboard(hWnd)){
        EmptyClipboard();
        SetClipboardData(CF_TEXT, (HANDLE)src);
        CloseClipboard();
    }
}
