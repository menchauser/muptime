/**
 * @author Muhamed Karanashev
 */
#define _UNICODE
#define WINVER 0x500
#include <windows.h>
#include <winuser.h>
#include <tchar.h>
#include "resource.h"
#include "main.h"
#include <memory>
using namespace std;
// TODO (Muhamed#1#): установить везде поддержку auto_ptr

#define UCALLBACKMESSAGE 0x1001
#define PICTOID 1025

static wchar_t szWindowClass[] = _T("uptimeAppClass"); /**< имя класса нашего окна */
static wchar_t szTitle[] = _T("muptime"); /**< заголовок (не нужен) нашего окна */
static BOOL isMinimized = FALSE; /**< флаг свертки/развертки окна в трей */
static UINT POS_X = 0; /**< координата окна по умолчанию */
static UINT POS_Y = 48; /**< координата окна по умолчанию */
static UINT HEIGHT = 22; /**< высота окна */
static UINT WIDTH = 78; /**< ширина окна */
static wchar_t iniFile[MAX_PATH]; /**< имя файла настроек (надо сделать локальной переменной) */
static LOGFONTW lg; /**< указатель на структуру с параметрами нашего шрифта*/
static HFONT fnt; /**< дескриптор нашего шрифта*/

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
    wnd.hbrBackground = (HBRUSH) (COLOR_WINDOWFRAME);
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
    WIDTH = GetPrivateProfileIntW(_T("Size"), _T("Width"), WIDTH, iniFile);
    HEIGHT = GetPrivateProfileIntW(_T("Size"), _T("Height"), HEIGHT, iniFile);
    HWND hWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP | WS_DLGFRAME,
        POS_X, POS_Y,
        WIDTH, HEIGHT,
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

//    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
//    SetLayeredWindowAttributes(hWnd, RGB(0xff, 0xff, 0xff) , 0, LWA_COLORKEY);

    SetWindowPos(hWnd, HWND_TOPMOST, POS_X, POS_Y, 0, 0, SWP_NOSIZE);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    //прячемся в трей
    {
        auto_ptr<NOTIFYICONDATAW> ntdata(new NOTIFYICONDATAW);
        ntdata->cbSize = sizeof(NOTIFYICONDATA);
        ntdata->hWnd = hWnd;
        ntdata->uID = PICTOID;//идентификатор пиктограммы на панели задач
        ntdata->uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
        ntdata->uCallbackMessage = UCALLBACKMESSAGE;
        ntdata->hIcon = LoadIconA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDI_ICON1));
        wcscpy(ntdata->szTip, L"µptime");
        Shell_NotifyIconW(NIM_ADD, ntdata.get());
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam) {
    const UINT_PTR IDT_TIMER1 = 1;

    PAINTSTRUCT ps;
    HDC hdc;
    switch(message) {
        case WM_PAINT:
            {
                //при перерисовке: отображаем текущий аптайм
                auto_ptr<wchar_t> time(new wchar_t[16]);
                hdc = BeginPaint(hWnd, &ps);
                SelectObject(hdc, fnt);
                getUptimeStr(time.get());
                drawUptime(hdc, time.get(), WIDTH, HEIGHT);
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_CREATE:
            //при создании: ставим таймер на обновление аптайма
            SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
            break;
        case WM_DESTROY:
            //при выходе: сохраняем текущее положение окна, чистим трей, убиваем таймер и уходим
            {
                auto_ptr<tagRECT> rect(new tagRECT);
                GetWindowRect(hWnd, rect.get());
                wchar_t pos_x[4];
                wchar_t pos_y[4];
                wchar_t width[4];
                wchar_t height[4];
                _ltow(rect->left, pos_x, 10);
                _ltow(rect->top, pos_y, 10);
                _ltow(WIDTH, width, 10);
                _ltow(HEIGHT, height, 10);
                WritePrivateProfileStringW(_T("Position"), _T("X"), pos_x, iniFile);
                WritePrivateProfileStringW(_T("Position"), _T("Y"), pos_y, iniFile);
                WritePrivateProfileStringW(_T("Size"), _T("Width"), width, iniFile);
                WritePrivateProfileStringW(_T("Size"), _T("Height"), height, iniFile);

                auto_ptr<NOTIFYICONDATA> ntdata(new NOTIFYICONDATA);
                ntdata->cbSize = sizeof(NOTIFYICONDATA);
                ntdata->hWnd = hWnd;
                ntdata->uID = PICTOID;//идентификатор пиктограммы на панели задач
                Shell_NotifyIcon(NIM_DELETE, ntdata.get());
            }
            KillTimer(hWnd, IDT_TIMER1);
            PostQuitMessage(0);
            break;
        case WM_TIMER:
            {
                //при тике таймера: обновляем и отображаем аптайм
                auto_ptr<wchar_t> time(new wchar_t[16]);
                hdc = GetDC(hWnd);
                SelectObject(hdc, fnt);
                getUptimeStr(time.get());
                drawUptime(hdc, time.get(), WIDTH, HEIGHT);
                ReleaseDC(hWnd, hdc);
            }
            break;
        case WM_KEYDOWN:
            //обрабатываем нажатия на клавиатуру
            if (wparam == VK_ESCAPE)
                SendMessage(hWnd, WM_DESTROY, wparam, lparam);
            if (wparam == 'q' || wparam == 'Q')
                SendMessage(hWnd, WM_SIZE, SIZE_MINIMIZED, NULL);
            if (wparam == 'c' || wparam == 'C') {
                auto_ptr<wchar_t> time(new wchar_t[16]);
                getUptimeStr(time.get());
                copyTextToClipboard(hWnd, time.get());
            }
            break;
        case WM_LBUTTONDOWN:
            //перетаскиваем окно программы левой кнопкой
            SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
            break;
        case WM_RBUTTONUP:
            {
                //копипастим текст по нажатию правой кнопки
                auto_ptr<wchar_t> time(new wchar_t[16]);
                getUptimeStr(time.get());
                copyTextToClipboard(hWnd, time.get());
            }
            break;
        case WM_MBUTTONUP:
            //выходим по клику средней кнопкой
            SendMessage(hWnd, WM_DESTROY, wparam, lparam);
            break;
        case WM_SIZE:
            //при приходе сообщения о ресайзинге (от обработчика в трее) - изменяем окно окно
            switch (wparam) {
                case SIZE_MINIMIZED:
                    ShowWindow(hWnd, SW_HIDE);
                    isMinimized = TRUE;
                    break;
                case SIZE_MAXIMIZED:
                    ShowWindow(hWnd, SW_SHOWNORMAL);
                    isMinimized = FALSE;
                    break;
                default:
                    break;
            }
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
                        {
                            auto_ptr<wchar_t> time(new wchar_t[16]);
                            getUptimeStr(time.get());
                            copyTextToClipboard(hWnd, time.get());
                        }
                        break;
                    //или выходим
                    case WM_MBUTTONUP:
                        SendMessage(hWnd, WM_DESTROY, wparam, lparam);
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_QUIT:
            //при выходе чистим за собой - удаляем шрифт
            DeleteObject(fnt);
            break;
        default:
            return DefWindowProc(hWnd, message, wparam, lparam);
            break;
    }
    return 0;
}
