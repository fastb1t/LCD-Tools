#include <tchar.h>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "wnd_proc.h"
#include "resource.h"

static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnDestroy(HWND);                                                        // WM_DESTROY

#define IDC_STATIC1 1001
#define IDC_STATIC2 1002

#define IDC_WIDTH 1003
#define IDC_HEIGHT 1004
#define IDC_DATA 1005
#define IDC_PROCESS_DATA 1006

static HBRUSH g_hBackgroundBrush = NULL;

static HFONT g_hDefaultFont = NULL;

static std::vector<uint8_t> g_data;

static size_t g_width = 0;
static size_t g_height = 0;

static HDC g_hImageDC = NULL;
static HBITMAP g_hImageBitmap = NULL;
static HBITMAP g_hImageOldBitmap = NULL;


// [to_dec]:
unsigned int to_dec(std::string str)
{
    unsigned int i = 0;

    str = str.substr(2, 2);

    std::stringstream ss;
    ss << std::hex << str;
    ss >> i;

    return i;
}
// [/to_dec]


// [WindowProcedure]: 
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);

    case WM_CTLCOLORSTATIC:
    {
        return (LRESULT)g_hBackgroundBrush;
    }
    break;

    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
// [/WindowProcedure]


// [OnCreate]: WM_CREATE
static BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    InitCommonControls();

    g_hBackgroundBrush = CreateSolidBrush(RGB(185, 190, 180));

    g_hDefaultFont = CreateFont(16, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));

    CreateWindowEx(0, _T("button"), _T("Data"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 10, 500, 500, hWnd, (HMENU)IDC_STATIC1, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_STATIC1), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    CreateWindowEx(0, _T("button"), _T("Preview"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        520, 10, 500, 500, hWnd, (HMENU)IDC_STATIC2, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_STATIC2), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    CreateWindowEx(0, _T("edit"), _T("20"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_NUMBER,
        100, 30, 100, 20, hWnd, (HMENU)IDC_WIDTH, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);
    Edit_LimitText(GetDlgItem(hWnd, IDC_WIDTH), 3);

    CreateWindowEx(0, _T("edit"), _T("20"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_NUMBER,
        100, 60, 100, 20, hWnd, (HMENU)IDC_HEIGHT, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);
    Edit_LimitText(GetDlgItem(hWnd, IDC_HEIGHT), 3);

    CreateWindowEx(0, _T("edit"), _T("\
0xFF, 0xFF, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0xFF, 0xFF,\r\n\
0xFF, 0x1F, 0x1F, 0x1E, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0xFF,\r\n\
0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F,\r\n\
0x1F, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1F, 0x1E, 0x1F, 0x1E, 0x1F, 0x1E, 0x1F,\r\n\
0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1E, 0x1F,\r\n\
0x1F, 0x1E, 0x1F, 0x1F, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1F, 0x1E, 0x1F, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1E,\r\n\
0x1F, 0x1F, 0x1E, 0x1F, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E,\r\n\
0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E,\r\n\
0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F,\r\n\
0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F,\r\n\
0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F,\r\n\
0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E,\r\n\
0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E,\r\n\
0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0xFF, 0x1E,\r\n\
0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0xFF, 0x1F,\r\n\
0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0xFF, 0xFF, 0x1E,\r\n\
0x1E, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x1E, 0x1E, 0x1E, 0xFF, 0xFF, 0xFF, 0x1F,\r\n\
0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1E,\r\n\
0xFF, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0xFF,\r\n\
0xFF, 0xFF, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1E, 0x1E, 0xFF, 0xFF\r\n\
        "), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | ES_MULTILINE,
        20, 90, 480, 410, hWnd, (HMENU)IDC_DATA, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_DATA), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    CreateWindowEx(0, _T("button"), _T(">>"), WS_CHILD | WS_VISIBLE,
        400, 30, 100, 50, hWnd, (HMENU)IDC_PROCESS_DATA, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_PROCESS_DATA), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    return TRUE;
}
// [/OnCreate]


// [OnCommand]: WM_COMMAND
static void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDC_PROCESS_DATA:
    {
        g_data.clear();

        g_width = 0;
        g_height = 0;

        int width = GetDlgItemInt(hWnd, IDC_WIDTH, NULL, FALSE);
        if (width > 0)
        {
            g_width = (size_t)width;
        }

        int height = GetDlgItemInt(hWnd, IDC_HEIGHT, NULL, FALSE);
        if (height > 0)
        {
            g_height = (size_t)height;
        }

        if (width > 0 && height > 0)
        {
            size_t size = GetWindowTextLength(GetDlgItem(hWnd, IDC_DATA));
            if (size > 0)
            {
                TCHAR* lpszText = new (std::nothrow) TCHAR[size + 2];
                if (lpszText != nullptr)
                {
                    memset(lpszText, 0, size + 1);
                    if (GetWindowText(GetDlgItem(hWnd, IDC_DATA), lpszText, size + 1) > 0)
                    {
                        std::string text = lpszText;

                        std::string byte;

                        bool bNewByte = false;

                        for (size_t i = 0; i < text.length(); i++)
                        {
                            if (text[i] == '\r' || text[i] == '\n')
                            {
                                continue;
                            }

                            if (!bNewByte)
                            {
                                if (text[i] == '0')
                                {
                                    bNewByte = true;
                                }
                            }

                            if (bNewByte)
                            {
                                byte.push_back(text[i]);
                            }

                            if (byte.length() == 4)
                            {
                                g_data.push_back(to_dec(byte));
                                byte.clear();
                                bNewByte = false;
                            }
                        }

                        HDC hDC = GetDC(hWnd);
                        if (hDC)
                        {
                            g_hImageDC = CreateCompatibleDC(hDC);
                            g_hImageBitmap = CreateCompatibleBitmap(hDC, g_width, g_height);
                            g_hImageOldBitmap = (HBITMAP)SelectObject(g_hImageDC, g_hImageBitmap);
                            ReleaseDC(hWnd, hDC);
                        }

                        size_t i = 0;
                        for (size_t y = 0; y < g_height; y++)
                        {
                            for (size_t x = 0; x < g_width; x++)
                            {
                                uint8_t byte = g_data[i++];

                                uint8_t r = 0;
                                uint8_t g = 0;
                                uint8_t b = 0;

                                //r = 36 * ((byte & ~0x1F) >> 5);
                                //g = 36 * ((byte & ~0xE3) >> 2);
                                //b = 85 * (byte & ~0xFC);
                                

                                //r = (byte >> 5) * 32;
                                //g = ((byte & 28) >> 2) * 32;
                                //b = (byte & 3) * 64;

                                //r = (byte >> 5) * (255 / 7);
                                //g = ((byte >> 2) & 0x07) * (255 / 7);
                                //b = (byte & 0x03) * (255 / 3);

                                r = (byte >> 5) * 32;
                                g = ((byte >> 2) << 3) * 32;
                                b = (byte << 6) * 64;

                                SetPixel(g_hImageDC, x, y, RGB(r, g, b));
                            }
                        }
                    }

                    delete[] lpszText;
                }
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;
    }
}
// [/OnCommand]


// [OnPaint]: WM_PAINT
static void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hWnd, &ps);
    RECT rc;
    GetClientRect(hWnd, &rc);
    const int iWindowWidth = rc.right - rc.left;
    const int iWindowHeight = rc.bottom - rc.top;
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hDC, iWindowWidth, iWindowHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    FillRect(hMemDC, &ps.rcPaint, g_hBackgroundBrush);

    int iOldBkMode = SetBkMode(hMemDC, TRANSPARENT);
    HFONT hOldFont = (HFONT)GetCurrentObject(hMemDC, OBJ_FONT);
    HBRUSH hOldBrush = (HBRUSH)GetCurrentObject(hMemDC, OBJ_BRUSH);
    HPEN hOldPen = (HPEN)GetCurrentObject(hMemDC, OBJ_PEN);
    COLORREF clrOldColor = GetTextColor(hMemDC);

    SelectObject(hMemDC, g_hDefaultFont);

    TCHAR szText[1024] = { 0 };
    SIZE size;

    lstrcpy(szText, _T("Width:"));
    TextOut(hMemDC, 20, 30, szText, lstrlen(szText));

    lstrcpy(szText, _T("Height:"));
    TextOut(hMemDC, 20, 60, szText, lstrlen(szText));

    if (g_hImageDC)
    {
        BitBlt(hMemDC, 530, 30, g_width, g_height, g_hImageDC, 0, 0, SRCCOPY);
    }

    lstrcpy(szText, _T("Copyright \251 2020 fastb1t"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(hMemDC, iWindowWidth - 10 - size.cx, iWindowHeight - 10 - size.cy, szText, lstrlen(szText));

    SetTextColor(hMemDC, clrOldColor);
    SelectObject(hMemDC, hOldPen);
    SelectObject(hMemDC, hOldBrush);
    SelectObject(hMemDC, hOldFont);
    SetBkMode(hMemDC, iOldBkMode);

    BitBlt(hDC, 0, 0, iWindowWidth, iWindowHeight, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    EndPaint(hWnd, &ps);
}
// [/OnPaint]


// [OnDestroy]: WM_DESTROY
static void OnDestroy(HWND hWnd)
{
    DeleteObject(g_hBackgroundBrush);

    DeleteObject(g_hDefaultFont);

    PostQuitMessage(0);
}
// [/OnDestroy]
