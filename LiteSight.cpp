// LiteSight.cpp : Light weight crosshair overlay application.
// Author : Matthew Masel

#define WINVER 0x0600        // Windows Vista or later
#define _WIN32_WINNT 0x0600  // Windows Vista or later

#include "framework.h"
#include "LiteSight.h"
#include <CommCtrl.h>
#include <shellapi.h>

#define MAX_LOADSTRING 100
#define WM_TRAYICON (WM_USER + 1)

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
COLORREF g_crosshairColor = RGB(0, 255, 0);
// Crosshair parameters
int g_crosshairLength = 0;
int g_crosshairGapSize = 0;
int g_crosshairThickness = 7;
int g_endcapStyle = PS_ENDCAP_ROUND;
// System Tray
NOTIFYICONDATA nid;
HMENU hTrayMenu;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SettingsProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES; // Adjust flags as necessary
    InitCommonControlsEx(&icex);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LITESIGHT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LITESIGHT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LITESIGHT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    // Modify hbrBackground to make window transparent
    wcex.hbrBackground  = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowExW(
       WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, 
       szWindowClass, 
       szTitle, 
       WS_POPUP, 
       0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
       nullptr, 
       nullptr, 
       hInstance, 
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // Set the transparency color key to black
   SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // Register the global hotkey (Ctrl + Alt + S)
   if (!RegisterHotKey(hWnd, 1, MOD_CONTROL | MOD_ALT, 'C'))
   {
       MessageBox(hWnd, L"Failed to register hotkey!", L"Error", MB_OK | MB_ICONERROR);
   }

   // System Tray
   // Initialize the NOTIFYICONDATA structure
   ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = hWnd;
   nid.uID = 1; // Unique ID for the icon
   nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
   nid.uCallbackMessage = WM_TRAYICON;
   nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LITESIGHT));
   wcscpy_s(nid.szTip, L"LiteSight");

   // Add the icon to the system tray
   Shell_NotifyIcon(NIM_ADD, &nid);

   // Create the context menu for the tray icon
   hTrayMenu = CreatePopupMenu();
   AppendMenu(hTrayMenu, MF_STRING, IDM_SETTINGS, L"Settings");
   AppendMenu(hTrayMenu, MF_SEPARATOR, 0, nullptr);
   AppendMenu(hTrayMenu, MF_STRING, IDM_EXIT, L"Exit");

   return TRUE;
}

INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LRESULT sliderValueColor = 75;
    static LRESULT sliderValueThickness = 7;
    static LRESULT sliderValueLength = 0;
    static LRESULT sliderValueGap = 0;
    static HWND hSliderColor = nullptr;
    static HWND hSliderThickness = nullptr;
    static HWND hSliderLength = nullptr;
    static HWND hSliderGap = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Initialize slider control color
        hSliderColor = GetDlgItem(hDlg, IDC_SLIDER_COLOR);
        SendMessage(hSliderColor, TBM_SETRANGE, TRUE, MAKELONG(0, 255)); // Set range from 0 to 255
        SendMessage(hSliderColor, TBM_SETPOS, TRUE, sliderValueColor); // Set initial position

        // Initialize slider control thickness
        hSliderThickness = GetDlgItem(hDlg, IDC_SLIDER_THICKNESS);
        SendMessage(hSliderThickness, TBM_SETRANGE, TRUE, MAKELONG(0, 10)); // Set range from 1 to 10
        SendMessage(hSliderThickness, TBM_SETPOS, TRUE, sliderValueThickness); // Set initial position

        // Initialize slider control length
        hSliderLength = GetDlgItem(hDlg, IDC_SLIDER_LENGTH);
        SendMessage(hSliderLength, TBM_SETRANGE, TRUE, MAKELONG(0, 60)); // Set range from 1 to 10
        SendMessage(hSliderLength, TBM_SETPOS, TRUE, sliderValueLength); // Set initial position

        // Initialize slider control gap
        hSliderGap = GetDlgItem(hDlg, IDC_SLIDER_GAP);
        SendMessage(hSliderGap, TBM_SETRANGE, TRUE, MAKELONG(0, 20)); // Set range from 1 to 10
        SendMessage(hSliderGap, TBM_SETPOS, TRUE, sliderValueGap); // Set initial position

        // Initialize crosshair edge type
        // Set default radio button selection
        HWND hRadio1 = GetDlgItem(hDlg, IDC_RADIO1);
        HWND hRadio2 = GetDlgItem(hDlg, IDC_RADIO2);

        // Choose the default button (e.g., round by default)
        SendMessage(hRadio1, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(hRadio2, BM_SETCHECK, BST_UNCHECKED, 0);
        
        return (INT_PTR)TRUE;
    }
    case WM_HSCROLL:
    {
        if ((HWND)lParam == hSliderColor)
        {
            if (LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_ENDTRACK)
            {
                // Slider value changed
                sliderValueColor = SendMessage(hSliderColor, TBM_GETPOS, 0, 0);

                // Map slider value to the specified color transitions
                COLORREF color;
                if (sliderValueColor < 43)
                {
                    // 255,0,0 to 255,255,0
                    color = RGB(255, sliderValueColor * 255 / 43, 0);
                }
                else if (sliderValueColor < 85)
                {
                    // 255,255,0 to 0,255,0
                    color = RGB(255 - ((sliderValueColor - 43) * 255 / 42), 255, 0);
                }
                else if (sliderValueColor < 128)
                {
                    // 0,255,0 to 0,255,255
                    color = RGB(0, 255, (sliderValueColor - 85) * 255 / 42);
                }
                else if (sliderValueColor < 170)
                {
                    // 0,255,255 to 0,0,255
                    color = RGB(0, 255 - ((sliderValueColor - 128) * 255 / 42), 255);
                }
                else if (sliderValueColor < 213)
                {
                    // 0,0,255 to 255,0,255
                    color = RGB((sliderValueColor - 170) * 255 / 42, 0, 255);
                }
                else
                {
                    // 255,0,255 to 255,0,0
                    color = RGB(255, 0, 255 - ((sliderValueColor - 213) * 255 / 42));
                }

                // Update the crosshair color
                g_crosshairColor = color;

                // Invalidate the main window to force it to redraw
                HWND hMainWnd = GetParent(hDlg);
                InvalidateRect(hMainWnd, NULL, TRUE);
            }
        }

        if ((HWND)lParam == hSliderThickness)
        {
            if (LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_ENDTRACK)
            {
                // Thickness slider value changed
                sliderValueThickness = SendMessage(hSliderThickness, TBM_GETPOS, 0, 0);

                // Update the global crosshair thickness
                g_crosshairThickness = static_cast<int>(sliderValueThickness);

                // Invalidate the main window to force it to redraw
                HWND hMainWnd = GetParent(hDlg);
                InvalidateRect(hMainWnd, NULL, TRUE);
            }
        }

        if ((HWND)lParam == hSliderLength)
        {
            if (LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_ENDTRACK)
            {
                // Thickness slider value changed
                sliderValueLength = SendMessage(hSliderLength, TBM_GETPOS, 0, 0);

                // Update the global crosshair thickness
                g_crosshairLength = static_cast<int>(sliderValueLength);

                // Invalidate the main window to force it to redraw
                HWND hMainWnd = GetParent(hDlg);
                InvalidateRect(hMainWnd, NULL, TRUE);
            }
        }

        if ((HWND)lParam == hSliderGap)
        {
            if (LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_ENDTRACK)
            {
                // Thickness slider value changed
                sliderValueGap = SendMessage(hSliderGap, TBM_GETPOS, 0, 0);

                // Update the global crosshair thickness
                g_crosshairGapSize = static_cast<int>(sliderValueGap);

                // Invalidate the main window to force it to redraw
                HWND hMainWnd = GetParent(hDlg);
                InvalidateRect(hMainWnd, NULL, TRUE);
            }
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RADIO1:
        case IDC_RADIO2:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, LOWORD(wParam));

                if (LOWORD(wParam) == IDC_RADIO1)
                {
                    g_endcapStyle = PS_ENDCAP_ROUND;
                }
                else if (LOWORD(wParam) == IDC_RADIO2)
                {
                    g_endcapStyle = PS_ENDCAP_SQUARE;
                }
                // Invalidate the main window to force it to redraw
                HWND hMainWnd = GetParent(hDlg);
                InvalidateRect(hMainWnd, NULL, TRUE);
            }
            return (INT_PTR)TRUE;
        case IDOK:
            // Apply settings if OK is pressed
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            // Cancel button pressed
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HOTKEY:
    {
        // Check if the hotkey is the one registered for settings (Ctrl + Shift + S)
        if (wParam == 1) // 1 is the ID used in RegisterHotKey
        {
            // Open the settings dialog
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsProc);
        }
        break;
    }
    case WM_TRAYICON:
        if (LOWORD(lParam) == WM_RBUTTONDOWN)
        {
            // Show the context menu
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
        }
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsProc);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            Shell_NotifyIcon(NIM_DELETE, &nid); // Remove the tray icon
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    // Draw the crosshair
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);

        // Fill the window with a solid color
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        // Set up pen with square end caps
        LOGBRUSH lb;
        lb.lbStyle = BS_SOLID;
        lb.lbColor = g_crosshairColor;
        lb.lbHatch = 0;

        HPEN hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | g_endcapStyle, g_crosshairThickness, &lb, 0, NULL);
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);

        // Center of the window
        int centerX = rect.right / 2;
        int centerY = rect.bottom / 2;

        // Draw horizontal lines
        MoveToEx(hdc, centerX - g_crosshairLength, centerY, NULL);
        LineTo(hdc, centerX - g_crosshairGapSize, centerY); // Left part of the horizontal line
        MoveToEx(hdc, centerX + g_crosshairGapSize, centerY, NULL); // Right part of the horizontal line
        LineTo(hdc, centerX + g_crosshairLength, centerY);

        // Draw vertical lines
        MoveToEx(hdc, centerX, centerY - g_crosshairLength, NULL);
        LineTo(hdc, centerX, centerY - g_crosshairGapSize); // Top part of the vertical line
        MoveToEx(hdc, centerX, centerY + g_crosshairGapSize, NULL); // Bottom part of the vertical line
        LineTo(hdc, centerX, centerY + g_crosshairLength);

        // Restore old pen and clean up
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid); // Remove the tray icon
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) 
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
