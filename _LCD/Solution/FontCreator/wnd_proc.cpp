#include <tchar.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "wnd_proc.h"

static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnDestroy(HWND);                                                        // WM_DESTROY

#define IDC_BYTE1 1001
#define IDC_BYTE2 1002
#define IDC_BYTE3 1003
#define IDC_BYTE4 1004
#define IDC_BYTE5 1005

#define IDC_PROCESS_DATA 1006

#define IDC_BYTE1_BIT1 2001
#define IDC_BYTE2_BIT1 3001
#define IDC_BYTE3_BIT1 4001
#define IDC_BYTE4_BIT1 5001
#define IDC_BYTE5_BIT1 6001

#define IDC_FIXED 7001

static HBRUSH g_hBackgroundBrush = NULL;

static HPEN g_hGrayPen = NULL;

static HFONT g_hDefaultFont = NULL;

static unsigned char g_byte1 = 0x00;
static unsigned char g_byte2 = 0x00;
static unsigned char g_byte3 = 0x00;
static unsigned char g_byte4 = 0x00;
static unsigned char g_byte5 = 0x00;

static HBITMAP g_hLogoBitmap = NULL;
static HBITMAP g_hLogoOldBitmap = NULL;
static HDC g_hLogoDC = NULL;
static BITMAP g_bmLogo;


// [to_hex]:
std::string to_hex(unsigned int i)
{
    std::stringstream ss;
    ss << _T("0x") << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << i;
    return ss.str();
}
// [/to_hex]


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


// [isValid]:
bool isValid(std::string str)
{
    if (str.length() != 4)
    {
        return false;
    }

    if (str[0] != '0' || str[1] != 'x')
    {
        return false;
    }

    if (std::tolower(str[2]) != 'a' &&
        std::tolower(str[2]) != 'b' &&
        std::tolower(str[2]) != 'c' &&
        std::tolower(str[2]) != 'd' &&
        std::tolower(str[2]) != 'e' &&
        std::tolower(str[2]) != 'f')
    {
        if (!std::isdigit(str[2]))
        {
            return false;
        }
    }

    if (std::tolower(str[3]) != 'a' &&
        std::tolower(str[3]) != 'b' &&
        std::tolower(str[3]) != 'c' &&
        std::tolower(str[3]) != 'd' &&
        std::tolower(str[3]) != 'e' &&
        std::tolower(str[3]) != 'f')
    {
        if (!std::isdigit(str[3]))
        {
            return false;
        }
    }

    return true;
}
// [/isValid]


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
        return (LRESULT)GetStockObject(WHITE_BRUSH);
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

    g_hBackgroundBrush = CreateSolidBrush(RGB(190, 220, 180));

    g_hGrayPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));

    g_hDefaultFont = CreateFont(16, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));

    for (size_t i = 0; i < 5; i++)
    {
        CreateWindowEx(0, _T("edit"), _T("0x00"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            60, 50 + i * 30, 50, 20, hWnd, (HMENU)(IDC_BYTE1 + i), lpcs->hInstance, NULL);

        Edit_LimitText(GetDlgItem(hWnd, IDC_BYTE1 + i), 4);
        SendMessage(GetDlgItem(hWnd, IDC_BYTE1 + i), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);
    }

    CreateWindowEx(0, _T("button"), _T(">>"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        15, 200, 95, 20, hWnd, (HMENU)IDC_PROCESS_DATA, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_PROCESS_DATA), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    CreateWindowEx(0, _T("button"), _T("Always on top"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        15, 270 + ((320 - 270) >> 1) - (14 >> 1), 100, 14, hWnd, (HMENU)IDC_FIXED, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_FIXED), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    for (size_t i = 0; i < 8; i++)
    {
        CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            170 + (50 >> 1) - (14 >> 1), 80 + i * 30 + (20 >> 1) - (14 >> 1), 14, 14, hWnd, (HMENU)(IDC_BYTE1_BIT1 + i), lpcs->hInstance, NULL);

        CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            170 + 50 + (50 >> 1) - (14 >> 1), 80 + i * 30 + (20 >> 1) - (14 >> 1), 14, 14, hWnd, (HMENU)(IDC_BYTE2_BIT1 + i), lpcs->hInstance, NULL);

        CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            170 + 2 * 50 + (50 >> 1) - (14 >> 1), 80 + i * 30 + (20 >> 1) - (14 >> 1), 14, 14, hWnd, (HMENU)(IDC_BYTE3_BIT1 + i), lpcs->hInstance, NULL);

        CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            170 + 3 * 50 + (50 >> 1) - (14 >> 1), 80 + i * 30 + (20 >> 1) - (14 >> 1), 14, 14, hWnd, (HMENU)(IDC_BYTE4_BIT1 + i), lpcs->hInstance, NULL);

        CreateWindowEx(0, _T("button"), _T(""), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            170 + 4 * 50 + (50 >> 1) - (14 >> 1), 80 + i * 30 + (20 >> 1) - (14 >> 1), 14, 14, hWnd, (HMENU)(IDC_BYTE5_BIT1 + i), lpcs->hInstance, NULL);
    }

    return TRUE;
}
// [/OnCreate]


// [OnCommand]: WM_COMMAND
static void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
    unsigned char mask;

    if (id >= IDC_BYTE1_BIT1 && id < IDC_BYTE1_BIT1 + 8)
    {
        mask = static_cast<unsigned char>(std::pow(2, id - IDC_BYTE1_BIT1));

        if (g_byte1 & mask)
        {
            g_byte1 &= ~mask;
        }
        else
        {
            g_byte1 |= mask;
        }

        InvalidateRect(hWnd, NULL, FALSE);

        SetWindowText(GetDlgItem(hWnd, IDC_BYTE1), to_hex(g_byte1).c_str());
    }

    if (id >= IDC_BYTE2_BIT1 && id < IDC_BYTE2_BIT1 + 8)
    {
        mask = static_cast<unsigned char>(std::pow(2, id - IDC_BYTE2_BIT1));

        if (g_byte2 & mask)
        {
            g_byte2 &= ~mask;
        }
        else
        {
            g_byte2 |= mask;
        }

        InvalidateRect(hWnd, NULL, FALSE);

        SetWindowText(GetDlgItem(hWnd, IDC_BYTE2), to_hex(g_byte2).c_str());
    }

    if (id >= IDC_BYTE3_BIT1 && id < IDC_BYTE3_BIT1 + 8)
    {
        mask = static_cast<unsigned char>(std::pow(2, id - IDC_BYTE3_BIT1));

        if (g_byte3 & mask)
        {
            g_byte3 &= ~mask;
        }
        else
        {
            g_byte3 |= mask;
        }

        InvalidateRect(hWnd, NULL, FALSE);

        SetWindowText(GetDlgItem(hWnd, IDC_BYTE3), to_hex(g_byte3).c_str());
    }

    if (id >= IDC_BYTE4_BIT1 && id < IDC_BYTE4_BIT1 + 8)
    {
        mask = static_cast<unsigned char>(std::pow(2, id - IDC_BYTE4_BIT1));

        if (g_byte4 & mask)
        {
            g_byte4 &= ~mask;
        }
        else
        {
            g_byte4 |= mask;
        }

        InvalidateRect(hWnd, NULL, FALSE);

        SetWindowText(GetDlgItem(hWnd, IDC_BYTE4), to_hex(g_byte4).c_str());
    }

    if (id >= IDC_BYTE5_BIT1 && id < IDC_BYTE5_BIT1 + 8)
    {
        mask = static_cast<unsigned char>(std::pow(2, id - IDC_BYTE5_BIT1));

        if (g_byte5 & mask)
        {
            g_byte5 &= ~mask;
        }
        else
        {
            g_byte5 |= mask;
        }

        InvalidateRect(hWnd, NULL, FALSE);

        SetWindowText(GetDlgItem(hWnd, IDC_BYTE5), to_hex(g_byte5).c_str());
    }

    switch (id)
    {
    case IDC_PROCESS_DATA:
    {
        TCHAR szText[16] = { 0 };

        memset(szText, 0, 16);
        GetDlgItemText(hWnd, IDC_BYTE1, szText, 16);
        if (isValid(szText))
        {
            g_byte1 = to_dec(szText);
        }
        else
        {
            MessageBox(hWnd, _T("Byte 1 is incorrect."), _T("Error!"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            SetWindowText(GetDlgItem(hWnd, IDC_BYTE1), _T("0x00"));
            g_byte1 = 0x00;
        }

        memset(szText, 0, 16);
        GetDlgItemText(hWnd, IDC_BYTE2, szText, 16);
        if (isValid(szText))
        {
            g_byte2 = to_dec(szText);
        }
        else
        {
            MessageBox(hWnd, _T("Byte 2 is incorrect."), _T("Error!"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            SetWindowText(GetDlgItem(hWnd, IDC_BYTE2), _T("0x00"));
            g_byte2 = 0x00;
        }

        memset(szText, 0, 16);
        GetDlgItemText(hWnd, IDC_BYTE3, szText, 16);
        if (isValid(szText))
        {
            g_byte3 = to_dec(szText);
        }
        else
        {
            MessageBox(hWnd, _T("Byte 3 is incorrectò."), _T("Error!"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            SetWindowText(GetDlgItem(hWnd, IDC_BYTE3), _T("0x00"));
            g_byte3 = 0x00;
        }

        memset(szText, 0, 16);
        GetDlgItemText(hWnd, IDC_BYTE4, szText, 16);
        if (isValid(szText))
        {
            g_byte4 = to_dec(szText);
        }
        else
        {
            MessageBox(hWnd, _T("Byte 4 is incorrect."), _T("Error!"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            SetWindowText(GetDlgItem(hWnd, IDC_BYTE4), _T("0x00"));
            g_byte4 = 0x00;
        }

        memset(szText, 0, 16);
        GetDlgItemText(hWnd, IDC_BYTE5, szText, 16);
        if (isValid(szText))
        {
            g_byte5 = to_dec(szText);
        }
        else
        {
            MessageBox(hWnd, _T("Byte 5 is incorrect."), _T("Error!"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            SetWindowText(GetDlgItem(hWnd, IDC_BYTE5), _T("0x00"));
            g_byte5 = 0x00;
        }

        for (size_t i = 0; i < 8; i++)
        {
            mask = static_cast<unsigned char>(std::pow(2, i));

            Button_SetCheck(GetDlgItem(hWnd, IDC_BYTE1_BIT1 + i), (g_byte1 & mask) ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hWnd, IDC_BYTE2_BIT1 + i), (g_byte2 & mask) ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hWnd, IDC_BYTE3_BIT1 + i), (g_byte3 & mask) ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hWnd, IDC_BYTE4_BIT1 + i), (g_byte4 & mask) ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hWnd, IDC_BYTE5_BIT1 + i), (g_byte5 & mask) ? BST_CHECKED : BST_UNCHECKED);
        }

        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case IDC_FIXED:
    {
        if (SendMessage(GetDlgItem(hWnd, IDC_FIXED), BM_GETCHECK, 0, 0L))
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        else
            SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

    TCHAR szText[1024] = { 0 };
    SIZE size;

    SelectObject(hMemDC, g_hDefaultFont);

    SelectObject(hMemDC, GetStockObject(WHITE_BRUSH));

    SelectObject(hMemDC, GetStockObject(BLACK_PEN));


    Rectangle(hMemDC, 15, 30, 110, 45);

    Rectangle(hMemDC, 5, 5, 120, 35);
    lstrcpy(szText, _T("Data Input/Output"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(
        hMemDC,
        5 + ((120 - 5) >> 1) - (size.cx >> 1),
        5 + ((35 - 5) >> 1) - (size.cy >> 1),
        szText,
        lstrlen(szText)
    );

    Rectangle(hMemDC, 5, 40, 120, 230);

    for (size_t i = 0; i < 5; i++)
    {
        lstrcpy(szText, _T("Byte "));
        lstrcat(szText, std::to_string(i + 1).c_str());
        GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
        TextOut(hMemDC, 15, 50 + i * 30 + 10 - (size.cy >> 1), szText, lstrlen(szText));
    }


    Rectangle(hMemDC, 15, 260, 110, 275);

    Rectangle(hMemDC, 5, 235, 120, 265);
    lstrcpy(szText, _T("Settings"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(
        hMemDC,
        5 + ((120 - 5) >> 1) - (size.cx >> 1),
        235 + ((265 - 235) >> 1) - (size.cy >> 1),
        szText,
        lstrlen(szText)
    );

    Rectangle(hMemDC, 5, 270, 120, 320);


    Rectangle(hMemDC, 135, 30, 410, 45);

    Rectangle(hMemDC, 125, 5, 420, 35);
    lstrcpy(szText, _T("Work Area"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(
        hMemDC,
        125 + ((420 - 125) >> 1) - (size.cx >> 1),
        5 + ((35 - 5) >> 1) - (size.cy >> 1),
        szText,
        lstrlen(szText)
    );

    Rectangle(hMemDC, 125, 40, 420, 320);

    for (size_t i = 0; i < 5; i++)
    {
        lstrcpy(szText, _T("Byte "));
        lstrcat(szText, std::to_string(i + 1).c_str());
        GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
        TextOut(hMemDC, 170 + 50 * i, 50, szText, lstrlen(szText));
    }

    for (size_t i = 0; i < 8; i++)
    {
        lstrcpy(szText, _T("Bit "));
        lstrcat(szText, std::to_string(i + 1).c_str());
        GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
        TextOut(hMemDC, 135, 80 + i * 30 + 10 - (size.cy >> 1), szText, lstrlen(szText));
    }


    Rectangle(hMemDC, 435, 30, 585, 45);

    Rectangle(hMemDC, 425, 5, 595, 35);
    lstrcpy(szText, _T("Preview"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(
        hMemDC,
        425 + ((595 - 425) >> 1) - (size.cx >> 1),
        5 + ((35 - 5) >> 1) - (size.cy >> 1),
        szText,
        lstrlen(szText)
    );

    Rectangle(hMemDC, 425, 40, 595, 300);
    
    SelectObject(hMemDC, GetStockObject(BLACK_BRUSH));

    unsigned char mask;

    for (size_t i = 0; i < 8; i++)
    {
        mask = static_cast<unsigned char>(std::pow(2, i));
        
        if (g_byte1 & mask)
        {
            Rectangle(hMemDC, 435, 50 + i * 30, 465, 80 + i * 30);
        }

        if (g_byte2 & mask)
        {
            Rectangle(hMemDC, 465, 50 + i * 30, 495, 80 + i * 30);
        }

        if (g_byte3 & mask)
        {
            Rectangle(hMemDC, 495, 50 + i * 30, 525, 80 + i * 30);
        }

        if (g_byte4 & mask)
        {
            Rectangle(hMemDC, 525, 50 + i * 30, 555, 80 + i * 30);
        }

        if (g_byte5 & mask)
        {
            Rectangle(hMemDC, 555, 50 + i * 30, 585, 80 + i * 30);
        }
    }


    SelectObject(hMemDC, g_hGrayPen);

    for (size_t i = 0; i < 8; i++)
    {
        MoveToEx(hMemDC, 170, 90 + i * 30, NULL);
        LineTo(hMemDC, 410, 90 + i * 30);
    }

    for (size_t i = 0; i < 5; i++)
    {
        MoveToEx(hMemDC, 170 + i * 50 + (50 >> 1), 70, NULL);
        LineTo(hMemDC, 170 + i * 50 + (50 >> 1), 310);
    }

    for (size_t i = 0; i < 9; i++)
    {
        MoveToEx(hMemDC, 435, 50 + i * 30, NULL);
        LineTo(hMemDC, 585, 50 + i * 30);
    }

    for (size_t i = 0; i < 6; i++)
    {
        MoveToEx(hMemDC, 435 + i * 30, 50, NULL);
        LineTo(hMemDC, 435 + i * 30, 290);
    }


    lstrcpy(szText, _T("Copyright \251 2020 fastb1t"));
    GetTextExtentPoint32(hMemDC, szText, lstrlen(szText), &size);
    TextOut(hMemDC, iWindowWidth - 5 - size.cx, iWindowHeight - 5 - size.cy, szText, lstrlen(szText));


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

    DeleteObject(g_hGrayPen);

    DeleteObject(g_hDefaultFont);

    PostQuitMessage(0);
}
// [/OnDestroy]
