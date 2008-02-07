/**
 * (c) Muhamed Karanashev, 2008
 */
#define _UNICODE
#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "main.h"

#define UCALLBACKMESSAGE 0x1001
#define PICTOID 1025

static wchar_t szWindowClass[] = _T("uptimeAppClass");
static wchar_t szTitle[] = _T("muptime");
static BOOL isMinimized = FALSE;
static UINT POS_X = 0;
static UINT POS_Y = 48;
static wchar_t iniFile[MAX_PATH];
static LOGFONTW lg;
static HFONT fnt;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nCmdShow) {
    WNDCLASSEXW wnd;
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
    wnd.lpszClassName = _T("uptimeAppClass");
    wnd.hIconSm = LoadIcon(wnd.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    if (!RegisterClassExW(&wnd)) {
        MessageBoxW(NULL,
            _T("Ошибка RegisterClassEx!"),
            _T("muptime"),
            NULL);
        return 1;
    }
    //читаем настройки из muptime.ini
    GetCurrentDirectoryW(MAX_PATH, iniFile);
    wcscat(iniFile, _T("\\muptime.ini"));
    POS_X = GetPrivateProfileIntW(_T("Position"), _T("X"), POS_X, iniFile);
    POS_Y = GetPrivateProfileIntW(_T("Position"), _T("Y"), POS_Y, iniFile);
    HWND hWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP | WS_DLGFRAME,
        POS_X, POS_Y,
        96, 25,
        NULL,
        NULL,
        hInst,
        NULL);
    if (!hWnd) {
        MessageBoxW(NULL,
            _T("Ошибка CreateWindowEx"),
            _T("muptime"),
            NULL);
        return 1;
    }
    //устанавливаем шрифт
    setFontProperties(&lg);
    fnt = CreateFontIndirectW(&lg);
    //ставим окно поверх всех
    SetWindowPos(hWnd, HWND_TOPMOST, POS_X, POS_Y, 0, 0, SWP_NOSIZE);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    //прячемся в трей
    NOTIFYICONDATA* ntdata = new NOTIFYICONDATA;
    ntdata->cbSize = sizeof(NOTIFYICONDATA);
    ntdata->hWnd = hWnd;
    ntdata->uID = PICTOID;//идентификатор пиктограммы на панели задач
    ntdata->uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
    ntdata->uCallbackMessage = UCALLBACKMESSAGE;
    ntdata->hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
    strcpy(ntdata->szTip, "muptime");
    Shell_NotifyIcon(NIM_ADD, ntdata);
    delete ntdata;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam) {
    const UINT_PTR IDT_TIMER1 = 1;
    wchar_t *time = new wchar_t[16];
    PAINTSTRUCT ps;
    HDC hdc;
    switch(message) {
        case WM_PAINT:
            //при перерисовке: отображаем текущий аптайм
            hdc = BeginPaint(hWnd, &ps);
            SelectObject(hdc, fnt);
            getUptimeStr(time);
            TextOutW(hdc, 3, 3, time, wcslen(time));
            EndPaint(hWnd, &ps);
            break;
        case WM_CREATE:
            //при создании: ставим таймер на обновление аптайма
            SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
            break;
        case WM_DESTROY:
            //при выходе: сохраняем текущее положение окна, чистим трей, убиваем таймер и уходим
            {
                LPRECT rect = new tagRECT;
                GetWindowRect(hWnd, rect);
                wchar_t x_pos[4];
                wchar_t y_pos[4];
                _ltow(rect->left, x_pos, 10);
                _ltow(rect->top, y_pos, 10);
                WritePrivateProfileStringW(_T("Position"), _T("X"), x_pos, iniFile);
                WritePrivateProfileStringW(_T("Position"), _T("Y"), y_pos, iniFile);
                delete rect;

                NOTIFYICONDATA *ntdata = new NOTIFYICONDATA;
                ntdata->cbSize = sizeof(NOTIFYICONDATA);
                ntdata->hWnd = hWnd;
                ntdata->uID = PICTOID;//идентификатор пиктограммы на панели задач
                Shell_NotifyIcon(NIM_DELETE, ntdata);
                delete ntdata;
            }
            KillTimer(hWnd, IDT_TIMER1);
            PostQuitMessage(0);
            break;
        case WM_TIMER:
            //при тике таймера: обновляем и отображаем аптайм
            hdc = GetDC(hWnd);
            SelectObject(hdc, fnt);
            getUptimeStr(time);
            TextOutW(hdc, 3, 3, time, wcslen(time));
            ReleaseDC(hWnd, hdc);
            break;
        case WM_CHAR:
            //обрабатываем нажатия на клавиатуру
            if (wparam == 27)
                SendMessage(hWnd, WM_DESTROY, wparam, lparam);
            if (wparam == 'q' || wparam == 'Q')
                SendMessage(hWnd, WM_SIZE, SIZE_MINIMIZED, NULL);
            break;
        case WM_LBUTTONDOWN:
            //перетаскиваем окно программы левой кнопкой
            SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
            break;
        case WM_RBUTTONUP:
            //копипастим текст по нажатию правой кнопки
            getUptimeStr(time);
            copyTextToClipboard(hWnd, time);
            break;
        case WM_MBUTTONUP:
            //выходим по клику средней кнопкой
            SendMessage(hWnd, WM_DESTROY, wparam, lparam);
            break;
        case WM_SIZE:
            //при приходе сообщения о ресайзинге (от обработчика в трее) - изменяем окно окно
            if (wparam == SIZE_MINIMIZED) {
                ShowWindow(hWnd, SW_HIDE);
                isMinimized = TRUE;
            }
            if (wparam == SIZE_MAXIMIZED) {
                ShowWindow(hWnd, SW_SHOWNORMAL);
                isMinimized = FALSE;
            }
            break;
        case UCALLBACKMESSAGE:
            if (wparam == PICTOID) {
                //приходит сообщение от обработчика в трее - проверяем и восстанавливаем/сворачиваем окно
                switch (lparam) {
                    case WM_LBUTTONDBLCLK:
                        if (isMinimized)
                            SendMessage(hWnd, WM_SIZE, SIZE_MAXIMIZED, NULL);
                        else
                            SendMessage(hWnd, WM_SIZE, SIZE_MINIMIZED, NULL);
                        break;
                    //или копипастим аптайм
                    case WM_RBUTTONUP:
                        getUptimeStr(time);
                        copyTextToClipboard(hWnd, time);
                        break;
                    //или выходим
                    case WM_MBUTTONUP:
                        SendMessage(hWnd, WM_DESTROY, wparam, lparam);
                        break;
                }
            }
            break;
        case WM_QUIT:
            DeleteObject(fnt);
            break;
        default:
            return DefWindowProc(hWnd, message, wparam, lparam);
            break;
    }
    delete[] time;
    return 0;
}
