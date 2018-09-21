#pragma once
// Install UI Header
// UI Functions
#include <windows.h>
#include <string>

// Global variables
static HWND g_buttonHWnd = NULL;
static HWND g_progressHWnd = NULL;
static bool g_installed = false;
static bool g_displayInfo = false;
static bool g_displayCompleteText = false;

// FUNCTION: CreateProgressBar(HWND parentHWnd, RECT parentRect, int count)
//
// PURPOSE: Creates the progress bar
//
// parentHWnd: the HWND of the window to add the progress bar to
// parentRect: the dimmensions of the parent window
// count: the number of objects to be iterated through in the progress bar
BOOL CreateProgressBar(HWND parentHWnd, RECT parentRect, int count);

// FUNCTION: LaunchButton(HWND parentHWnd, RECT parentRect)
//
// PURPOSE: Create the lower right button
// 
// parentHWnd: the HWND of the window to add the button to
// parentRect: the specs of the parent window
BOOL LaunchButton(HWND parentHWnd, RECT parentRect);

// FUNCTION: ChangeButtonText(LPARAM newMessage)
//
// PURPOSE: Changes the text of the lower right button
//
// newMessage: the message to change the button to
BOOL ChangeButtonText(const std::wstring& newMessage);

// FUNCTION: HideButtonWindow()
//
// PURPOSE: Hides the lower right button
//
BOOL HideButtonWindow();

// FUNCTION: ChangeText(HWND parentHWnd, std::wstring& windowText)
//
// PURPOSE: Change the text of the installation window based on the given input
//
// parentHWnd: the HWND of the window to be changed
// windowText: the text to change the window to
BOOL ChangeText(HWND parentHWnd, std::wstring displayText, std::wstring  messageText, IStream* logoStream = nullptr);

// FUNCTION: CreateInitWindow(HINSTANCE hInstance, int nCmdShow, TCHAR windowClass[], TCHAR windowTitle[])
//
// PURPOSE: Creates the initial installation UI window
// windowClass: the class text of th
int CreateInitWindow(HINSTANCE hInstance, int nCmdShow, const std::wstring& windowClass, const std::wstring& title);

// FUNCTION: UpdateProgressBar
//
// PURPOSE: Increment the progress bar one tick based on preset tick
void UpdateProgressBar();
