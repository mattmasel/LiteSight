// LiteSight.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "LiteSight.h"
#include <CommCtrl.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
COLORREF g_crosshairColor = RGB(0, 255, 0);
// Crosshair parameters
int g_crosshairLength = 15;
int g_crosshairGapSize = 5;
int g_crosshairThickness = 3;

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
       WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, 
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

   return TRUE;
}

INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LRESULT sliderValueColor = 0;
    static LRESULT sliderValueThickness = 3;
    static HWND hSliderColor = nullptr;
    static HWND hSliderThickness = nullptr;

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
                g_crosshairThickness = sliderValueThickness;

                // Debug output
                wchar_t debugOutput[100];
                swprintf_s(debugOutput, 100, L"New thickness: %d\n", g_crosshairThickness);
                OutputDebugString(debugOutput);

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
        case IDOK:
            // Apply settings if OK is pressed
            // Optionally save sliderValueColor to application state
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
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
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
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    // This is where we draw the crosshair!
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);

        // Fill the window with a solid color
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0)); // Change this to your window's background color
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        // Set up pen
        HPEN hPen = CreatePen(PS_SOLID, g_crosshairThickness, g_crosshairColor);
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);

        // Center of the window
        int centerX = rect.right / 2;
        int centerY = rect.bottom / 2;

        // Draw horizontal lines with square edges
        MoveToEx(hdc, centerX - g_crosshairLength, centerY, NULL);
        LineTo(hdc, centerX - g_crosshairGapSize, centerY); // Left part of the horizontal line
        MoveToEx(hdc, centerX + g_crosshairGapSize, centerY, NULL); // Right part of the horizontal line
        LineTo(hdc, centerX + g_crosshairLength, centerY);

        // Draw vertical lines with square edges
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
    case WM_KEYDOWN:
        // Handle keyboard shortcuts here
        switch (wParam)
        {
        case 'C':
            if (GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState(VK_MENU) & 0x8000)
            {
                // Ctrl + Alt + C shortcut
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsProc);
                return 0; // Ensure to return 0 to indicate message handled
            }
            break;
        }
        break;
    case WM_DESTROY:
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
