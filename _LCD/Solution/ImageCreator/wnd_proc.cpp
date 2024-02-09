#include <tchar.h>
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "wnd_proc.h"
#include "resource.h"

static BOOL OnCreate(HWND, LPCREATESTRUCT);                                     	// WM_CREATE
static void OnCommand(HWND, int, HWND, UINT);                                      	// WM_COMMAND
static void OnPaint(HWND);                                                        	// WM_PAINT
static void OnDestroy(HWND);                                                        // WM_DESTROY

#define IDC_S_INPUT_IMAGE 1001
#define IDC_S_OUTPUT_IMAGE 1002

static HBRUSH g_hBackgroundBrush = NULL;

static HFONT g_hDefaultFont = NULL;

struct Image {
    HDC hDC;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    SIZE size;
};

static Image g_image;


static void ClearImageData(Image* image)
{
    if (image != nullptr)
    {
        if (image->hDC && image->hOldBitmap)
        {
            SelectObject(image->hDC, image->hOldBitmap);
        }

        if (image->hBitmap)
        {
            DeleteObject(image->hBitmap);
        }

        if (image->hDC)
        {
            DeleteDC(image->hDC);
        }

        image->hDC = NULL;
        image->hBitmap = NULL;
        image->hOldBitmap = NULL;
        image->size.cx = 0;
        image->size.cy = 0;
    }
}


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

    g_hBackgroundBrush = CreateSolidBrush(RGB(190, 195, 190));

    g_hDefaultFont = CreateFont(16, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));

    CreateWindowEx(0, _T("button"), _T("Input Image"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 10, 400, 400, hWnd, (HMENU)IDC_S_INPUT_IMAGE, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_S_INPUT_IMAGE), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    CreateWindowEx(0, _T("button"), _T("Output Image"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        420, 10, 400, 400, hWnd, (HMENU)IDC_S_OUTPUT_IMAGE, lpcs->hInstance, NULL);
    SendMessage(GetDlgItem(hWnd, IDC_S_OUTPUT_IMAGE), WM_SETFONT, (WPARAM)g_hDefaultFont, 0L);

    return TRUE;
}
// [/OnCreate]


// [OnCommand]: WM_COMMAND
static void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDM_OPEN:
    {
        TCHAR szFileName[1024] = { 0 };
        memset(szFileName, 0, sizeof(szFileName));

        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(OPENFILENAME));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = sizeof(szFileName);
        ofn.lpstrFilter = _T("Bitmaps (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0\0");
        ofn.nFilterIndex = 0;
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

        if (GetOpenFileName(&ofn))
        {
            ClearImageData(&g_image);

            HDC hDC = GetDC(hWnd);
            if (hDC)
            {
                g_image.hBitmap = (HBITMAP)LoadImage(NULL, _T(ofn.lpstrFile), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                g_image.hDC = CreateCompatibleDC(hDC);
                g_image.hOldBitmap = (HBITMAP)SelectObject(g_image.hDC, g_image.hBitmap);

                BITMAP bm;
                memset(&bm, 0, sizeof(BITMAP));

                GetObject(g_image.hBitmap, sizeof(BITMAP), &bm);

                g_image.size.cx = bm.bmWidth;
                g_image.size.cy = bm.bmHeight;

                ReleaseDC(hWnd, hDC);
            }

            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
    break;

    case IDM_EXIT:
    {
        DestroyWindow(hWnd);
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

    if (g_image.hDC)
    {
        BitBlt(hMemDC, 20, 30, g_image.size.cx, g_image.size.cy, g_image.hDC, 0, 0, SRCCOPY);
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

    ClearImageData(&g_image);

    PostQuitMessage(0);
}
// [/OnDestroy]
