// UI Functions

#include "InstallUI.h"
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <sys/types.h>
#include <thread>

// MSIXWindows.hpp define NOMINMAX because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. Use std namespace *BEFORE* including it.
using namespace std;
#include <GdiPlus.h>

Gdiplus::Image* g_image = nullptr;

static const int g_width = 500;  // width of window
static const int g_heigth = 400; // height of window

// FUNCTION: CreateProgressBar(HWND parentHWnd, RECT parentRect, int count)
//
// PURPOSE: Creates the progress bar
//
// parentHWnd: the HWND of the window to add the progress bar to
// parentRect: the dimmensions of the parent window
// count: the number of objects to be iterated through in the progress bar
BOOL CreateProgressBar(HWND parentHWnd, RECT parentRect, int count)
{
    int scrollHeight = GetSystemMetrics(SM_CYVSCROLL);

    // Creates progress bar on window
    g_progressHWnd = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        (LPTSTR)NULL,
        WS_CHILD | WS_VISIBLE,
        parentRect.left + 50, // x coord
        parentRect.bottom - scrollHeight - 125, // y coord
        parentRect.right - 100, // width
        scrollHeight, // height
        parentHWnd, // parent
        (HMENU)0,
        NULL,
        NULL
    );

    // Set defaults for range and increments
    SendMessage(g_progressHWnd, PBM_SETRANGE, 0, MAKELPARAM(0, count)); // set range
    SendMessage(g_progressHWnd, PBM_SETSTEP, (WPARAM)1, 0); // set increment
    return TRUE;
}

// FUNCTION: LaunchButton(HWND parentHWnd, RECT parentRect)
//
// PURPOSE: Create the lower right button
// 
// parentHWnd: the HWND of the window to add the button to
// parentRect: the specs of the parent window
BOOL LaunchButton(HWND parentHWnd, RECT parentRect) {
    LPVOID buttonPointer = nullptr;
    g_buttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        L"Install",  // text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
        parentRect.right - 100 - 50, // x coord
        parentRect.bottom - 50 - 50,  // y coord
        100,  // width
        50,  // height
        parentHWnd,  // parent
        NULL, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        buttonPointer); // pointer to button
    return TRUE;
}

// FUNCTION: ChangeButtonText(LPARAM newMessage)
//
// PURPOSE: Changes the text of the lower right button
//
// newMessage: the message to change the button to
BOOL ChangeButtonText(const std::wstring& newMessage)
{
    SendMessage(g_buttonHWnd, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(newMessage.c_str()));
    return ShowWindow(g_buttonHWnd, SW_SHOW);
}

BOOL HideButtonWindow()
{
    return ShowWindow(g_buttonHWnd, SW_HIDE);
}

// FUNCTION: ChangeText(HWND parentHWnd, std::wstring& windowText)
//
// PURPOSE: Change the text of the installation window based on the given input
//
// parentHWnd: the HWND of the window to be changed
// windowText: the text to change the window to
BOOL ChangeText(HWND parentHWnd, std::wstring displayName, std::wstring messageText, IStream* logoStream)
{
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(parentHWnd, &paint);

    Gdiplus::Graphics graphics(deviceContext);

    Gdiplus::RectF layoutRect(50, 50, g_width - 144, 100);
    Gdiplus::Font displayNameFont(L"Arial", 16);
    Gdiplus::Font messageFont(L"Arial", 10);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentNear);
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));

    graphics.DrawString(displayName.c_str(), -1, &displayNameFont, layoutRect, &format, &blackBrush);
    layoutRect.Y += 40;
    graphics.DrawString(messageText.c_str(), -1, &messageFont, layoutRect, &format, &blackBrush);

    if (logoStream != nullptr)
    {
        // We shouldn't fail if the image can't be loaded, just don't show it.
        g_image = Gdiplus::Image::FromStream(logoStream, FALSE);
            
    }
        
    if (g_image != nullptr)
    {
        Gdiplus::Status status = graphics.DrawImage(g_image, g_width - 200, 25);
    }

    EndPaint(parentHWnd, &paint);

    return TRUE;
}

// FUNCTION: CreateInitWindow(HINSTANCE hInstance, int nCmdShow, TCHAR windowClass[], TCHAR windowTitle[])
//
// PURPOSE: Creates the initial installation UI window
// windowClass: the class text of the window
// windowTitle: the window title
int CreateInitWindow(HINSTANCE hInstance, int nCmdShow, const std::wstring& windowClass, const std::wstring& title)
{
    HWND  hWnd = CreateWindow(
        const_cast<wchar_t*>(windowClass.c_str()),
        const_cast<wchar_t*>(title.c_str()),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_width,  // width of window
        g_heigth, // height of window
        NULL, // A handle to the parent or owner window of the window being created
        NULL, // a handle to a menu, or specifies a child-window identifier depending on the window style
        hInstance, // a handle to the instance o the module to be associated with the window
        NULL // Pointer to the window through the CREATESTRUCT structure
    );

    if (!hWnd)
    {
        MessageBox(NULL, L"Call to CreateWindow failed!", title.c_str(), NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

// FUNCTION: UpdateProgressBar
//
// PURPOSE: Increment the progress bar one tick based on preset tick
void UpdateProgressBar()
{
    SendMessage(g_progressHWnd, PBM_STEPIT, 0, 0);
}
