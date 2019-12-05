#include "InstallUI.hpp"
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <sys/types.h>
#include <thread>
#include <shldisp.h>
#include <shlobj.h>
#include <exdisp.h>
#include <stdlib.h>

#include <algorithm>
#include <sstream>
#include <iostream>
#include "resource.h"
#include <filesystem>

#include "Util.hpp"
#include "msixmgrLogger.hpp"
#include "MsixErrors.hpp"

// MSIXWindows.hpp defines NOMINMAX and undefines min and max because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. We can't use namespace std because c++17 defines std::byte, which conflicts with ::byte
#define max std::max
#define min std::min
#include <GdiPlus.h>
using namespace std;
using namespace MsixCoreLib;


static const int g_width = 500;  // width of window
static const int g_height = 400; // height of window

//
// PURPOSE: This compiles the information displayed on the UI when the user selects an msix
//
// hWnd: the HWND of the window to draw controls
// windowRect: the size of the window

HRESULT UI::DrawPackageInfo(HWND hWnd, RECT windowRect)
{
    auto messageText = GetStringResource(IDS_STRING_PUBLISHER) + m_publisherCommonName + L"\n" + GetStringResource(IDS_STRING_VERSION) + m_version;

    if (SUCCEEDED(m_loadingPackageInfoCode))
    {
        auto displayText = m_installOrUpdateText + L" " + m_displayName + L"?";
        ChangeText(hWnd, displayText, messageText, m_logoStream.get());
        ShowWindow(g_checkboxHWnd, SW_SHOW); //Show launch checkbox
        ShowWindow(g_buttonHWnd, SW_SHOW); //Show install button
    }
    else
    {
        auto displayText = m_displayName + L" " + GetStringResource(IDS_STRING_LOADING_PACKAGE_ERROR);
        ChangeText(hWnd, displayText, messageText, m_logoStream.get());
        ShowWindow(g_staticErrorTextHWnd, SW_SHOW); //Show error text heading
        ShowWindow(g_staticErrorDescHWnd, SW_SHOW); //Show error description
        std::wstringstream errorDescription;
        errorDescription << m_displayErrorString;
        SetWindowText(g_staticErrorDescHWnd, errorDescription.str().c_str());
    }
    return S_OK;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UI* ui = (UI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    RECT windowRect;
    GetClientRect(hWnd, &windowRect);
    switch (message)
    {
    case WM_CREATE:
        ui->CreateCheckbox(hWnd, windowRect);
        ui->InstallButton(hWnd, windowRect);
        ui->CreateLaunchButton(hWnd, windowRect, 275, 60);
        ui->CreateDisplayPercentageText(hWnd, windowRect);
        ui->CreateDisplayErrorText(hWnd, windowRect);
        break;
    case WM_PAINT:
    {
        if (ui != NULL)
        {
            ui->DrawPackageInfo(hWnd, windowRect);
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_INSTALLBUTTON:
        {
            g_installing = true;
            DestroyWindow(g_LaunchbuttonHWnd);
            DestroyWindow(g_buttonHWnd);
            ui->CreateCancelButton(hWnd, windowRect);
            UpdateWindow(hWnd);
            ui->CreateProgressBar(hWnd, windowRect);
            ShowWindow(g_progressHWnd, SW_SHOW); //Show progress bar only when install is clicked
            ui->ButtonClicked();
        }
        break;
        case IDC_LAUNCHCHECKBOX:
        {
            if (SendMessage(GetDlgItem(hWnd, IDC_LAUNCHCHECKBOX), BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                g_launchCheckBoxState = true;
            }
            else
            {
                g_launchCheckBoxState = false;
            }
        }
        break;
        case IDC_CANCELBUTTON:
        {
            ui->ConfirmAppCancel(hWnd);
            break;
        }
        case IDC_LAUNCHBUTTON:
        {
            ui->LaunchInstalledApp();
            ui->CloseUI();
            break;
        }
        }
        break;
    case WM_INSTALLCOMPLETE_MSG:
    {
        g_installing = false; // installation complete, clicking on 'x' should not show the cancellation popup
        if (g_launchCheckBoxState)
        {
            ui->LaunchInstalledApp(); // launch app
            ui->CloseUI();
        }
        else
        {
            RECT windowRect;
            GetClientRect(hWnd, &windowRect);
            DestroyWindow(g_CancelbuttonHWnd);
            ui->CreateLaunchButton(hWnd, windowRect, 150, 60);
            UpdateWindow(hWnd);
            ShowWindow(g_progressHWnd, SW_HIDE); //hide progress bar
            ShowWindow(g_checkboxHWnd, SW_HIDE); //hide launch check box
            ShowWindow(g_percentageTextHWnd, SW_HIDE);
            ShowWindow(g_staticPercentText, SW_HIDE);
            ShowWindow(g_LaunchbuttonHWnd, SW_SHOW);
        }
    }
    break;
    case WM_CTLCOLORSTATIC:
    {
        HBRUSH hbr = (HBRUSH)DefWindowProc(hWnd, message, wParam, lParam);
        ::DeleteObject(hbr);
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)::GetStockObject(NULL_BRUSH);
        break;
    }
    case WM_CLOSE:
        //show popup asking if user wants to stop installation only during app installation
        if (g_installing)
        {
            ui->ConfirmAppCancel(hWnd);
        }
        else
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_SIZE:
    case WM_SIZING:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        exit(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

void UI::ConfirmAppCancel(HWND hWnd)
{
    const int cancelResult = MessageBox(hWnd, m_cancelPopUpMessage.c_str(), m_cancelPopUpTitle.c_str(), MB_YESNO);
    switch (cancelResult)
    {
    case IDYES:
        m_msixResponse->CancelRequest();
        break;
    case IDNO:
        break;
    }
}

// Avoid launching the app elevated, we need to ShellExecute from Explorer in order to launch as the currently logged in user.
// See https://devblogs.microsoft.com/oldnewthing/?p=2643 
// ExecInExplorer.cpp sample from https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/dd940355(v=vs.85)
HRESULT GetDesktopAutomationObject(REFIID riid, void **ppv)
{
    ComPtr<IShellWindows> shellWindows;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&shellWindows)));

    HWND hwnd;
    ComPtr<IDispatch> dispatch;
    VARIANT vEmpty = {}; // VT_EMPTY
    RETURN_IF_FAILED(shellWindows->FindWindowSW(&vEmpty /*varLoc*/, &vEmpty /*varLocRoot*/, SWC_DESKTOP, (long*)&hwnd, SWFO_NEEDDISPATCH, &dispatch));

    ComPtr<IServiceProvider> serviceProvider;
    RETURN_IF_FAILED(dispatch->QueryInterface(IID_PPV_ARGS(&serviceProvider)));

    ComPtr<IShellBrowser> shellBrowser;
    RETURN_IF_FAILED(serviceProvider->QueryService(SID_STopLevelBrowser, &shellBrowser));

    ComPtr<IShellView> shellView;
    RETURN_IF_FAILED(shellBrowser->QueryActiveShellView(&shellView));

    ComPtr<IDispatch> dispatchView;
    RETURN_IF_FAILED(shellView->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&dispatchView)));
    RETURN_IF_FAILED(dispatchView->QueryInterface(riid, ppv));
    return S_OK;
}

HRESULT ShellExecuteFromExplorer(ExecutionInfo* executionInfo)
{
    ComPtr<IShellFolderViewDual> folderView;
    RETURN_IF_FAILED(GetDesktopAutomationObject(IID_PPV_ARGS(&folderView)));
    ComPtr<IDispatch> dispatch;
    RETURN_IF_FAILED(folderView->get_Application(&dispatch));

    Bstr bstrfile(executionInfo->resolvedExecutableFilePath.c_str());
    Bstr bstrArguments(executionInfo->commandLineArguments.c_str());
    VARIANT vtArguments = {};
    if (!executionInfo->commandLineArguments.empty())
    {
        vtArguments.vt = VT_BSTR;
        vtArguments.bstrVal = bstrArguments;
    }

    Bstr bstrWorkingDirectory(executionInfo->workingDirectory.c_str());
    VARIANT vtDirectory = {};
    if (!executionInfo->workingDirectory.empty())
    {
        vtDirectory.vt = VT_BSTR;
        vtDirectory.bstrVal = bstrWorkingDirectory;
    }

    VARIANT vtEmpty = {}; // VT_EMPTY
    ComPtr<IShellDispatch2> shellDispatch;
    RETURN_IF_FAILED(dispatch->QueryInterface(IID_PPV_ARGS(&shellDispatch)));
    RETURN_IF_FAILED(shellDispatch->ShellExecute(bstrfile, vtArguments, vtDirectory, vtEmpty /*operation*/, vtEmpty /*show*/));
    return S_OK;
}


HRESULT UI::LaunchInstalledApp()
{
    shared_ptr<IInstalledPackage> installedPackage;
    RETURN_IF_FAILED(m_packageManager->FindPackage(m_packageInfo->GetPackageFullName(), installedPackage));

    HRESULT hrShellExecute = ShellExecuteFromExplorer(installedPackage->GetExecutionInfo());
    if (FAILED(hrShellExecute))
    {
        TraceLoggingWrite(g_MsixUITraceLoggingProvider,
            "ShellExecute Failed",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrShellExecute, "HR"));
    }
    return S_OK;
}

void StartParseFile(HWND hWnd)
{
    int result = 0;

    if (result != 0)
    {
        std::cout << "Error: " << std::hex << result << " while extracting the appx package" << std::endl;
        Text<char> text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {
            std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {
            std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }
}

void CommandFunc(HWND hWnd, RECT windowRect) {
    std::thread t1(StartParseFile, hWnd);
    t1.detach();
    return;
}

void StartUIThread(UI* ui)
{
    // Free the console that we started with
    FreeConsole();

    // Register WindowClass and create the window
    HINSTANCE hInstance = GetModuleHandle(NULL);

    std::wstring windowClass = GetStringResource(IDS_STRING_UI_TITLE);
    std::wstring title = GetStringResource(IDS_STRING_UI_TITLE);

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONBIG));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = windowClass.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONSMALL));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"Call to RegisterClassEx failed!", title.c_str(), NULL);
        return;
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    ui->CreateInitWindow(hInstance, SW_SHOWNORMAL, windowClass, title);
    Gdiplus::GdiplusShutdown(gdiplusToken);

}

HRESULT UI::ParseInfoFromPackage()
{
    if (m_packageInfo == nullptr)
    {
        switch (m_type)
        {
        case InstallUIAdd:
        {
            HRESULT hrGetMsixPackageInfo = m_packageManager->GetMsixPackageInfo(m_path, m_packageInfo, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL);
            if (hrGetMsixPackageInfo == static_cast<HRESULT>(MSIX::Error::MissingAppxSignatureP7X))
            {
                TraceLoggingWrite(g_MsixUITraceLoggingProvider,
                    "Error - Signature missing from package, calling api again with signature skip validation parameter",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(hrGetMsixPackageInfo, "HR"));

                RETURN_IF_FAILED(m_packageManager->GetMsixPackageInfo(m_path, m_packageInfo, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE));
                m_displayErrorString = L"Ask the app developer for a new app package. This one isn't signed with a trusted certificate (0x8bad0031)";
                SetDisplayInfo();
                return static_cast<HRESULT>(MSIX::Error::MissingAppxSignatureP7X);
            }
            else if (hrGetMsixPackageInfo == static_cast<HRESULT>(MSIX::Error::CertNotTrusted))
            {
                TraceLoggingWrite(g_MsixUITraceLoggingProvider,
                    "Error - Certificate is not trusted, calling api again with signature skip validation parameter",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(hrGetMsixPackageInfo, "HR"));

                RETURN_IF_FAILED(m_packageManager->GetMsixPackageInfo(m_path, m_packageInfo, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE));
                m_displayErrorString = L"Either you need a new certificate installed for this app package, or you need a new app package with trusted certificates. Your system administrator or the app developer can help. A certificate chain processed, but terminated in a root certificate which isn't trusted (0x8bad0042)";
                SetDisplayInfo();
                return static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
            }
            else if (hrGetMsixPackageInfo == static_cast<HRESULT>(MSIX::Error::FileOpen))
            {
                /*TraceLoggingWrite(g_MsixUITraceLoggingProvider,
                    "Error - Invalid or corrupt File error, calling api again with signature skip validation parameter",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(hrGetMsixPackageInfo, "HR"));

                RETURN_IF_FAILED(m_packageManager->GetMsixPackageInfo(m_path, m_packageInfo, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE));
                m_displayErrorString = L"The package is either corrupted or invalid.";
                SetDisplayInfo();
                return static_cast<HRESULT>(MSIX::Error::FileOpen);*/
            }
            else
            {
                if (FAILED(hrGetMsixPackageInfo))
                {
                    SetDisplayInfo();
                    return hrGetMsixPackageInfo;
                }
            }
        }
        break;
        case InstallUIRemove:
        {
            shared_ptr<IInstalledPackage> installedPackage;
            RETURN_IF_FAILED(m_packageManager->FindPackage(m_path, installedPackage));
            m_packageInfo = installedPackage;
        }
        break;
        }
    }

    SetDisplayInfo();

    return S_OK;
}

void UI::SetDisplayInfo()
{
    // Obtain publisher name
    m_publisherCommonName = m_packageInfo->GetPublisherDisplayName();

    // Obtain version number
    m_version = m_packageInfo->GetVersion();

    //Obtain the number of files
    m_displayName = m_packageInfo->GetDisplayName();
    m_logoStream = std::move(m_packageInfo->GetLogo());

    //Obtain package capabilities
    m_capabilities = m_packageInfo->GetCapabilities();
}

HRESULT UI::ShowUI()
{
    m_loadingPackageInfoCode = ParseInfoFromPackage();

    std::thread thread(StartUIThread, this);
    thread.detach();

    DWORD waitResult = WaitForSingleObject(m_closeUI, INFINITE);

    return S_OK;
}

void UI::PreprocessRequest()
{
    if (FAILED(m_loadingPackageInfoCode))
    {
        return;
    }

    std::shared_ptr<IInstalledPackage> existingPackage;
    HRESULT hr = m_packageManager->FindPackageByFamilyName(m_packageInfo->GetPackageFamilyName(), existingPackage);
    if (existingPackage != nullptr && SUCCEEDED(hr))
    {
        if (CaseInsensitiveEquals(existingPackage->GetPackageFullName(), m_packageInfo->GetPackageFullName()))
        {
            /// Same package is already installed
            ChangeInstallButtonText(GetStringResource(IDS_STRING_REINSTALLAPP)); /// change install button text to 'reinstall'
            ShowWindow(g_LaunchbuttonHWnd, SW_SHOW); /// show launch button window
        }
        else
        {
            /// Package with same family name exists and may be an update
            m_installOrUpdateText = GetStringResource(IDS_STRING_UPDATETEXT);
            m_cancelPopUpMessage = GetStringResource(IDS_STRING_CANCEL_UPDATEPOPUP);
            ChangeInstallButtonText(GetStringResource(IDS_STRING_UPDATETEXT));
        }
    }
}

BOOL UI::CreateProgressBar(HWND parentHWnd, RECT parentRect)
{
    int scrollHeight = GetSystemMetrics(SM_CYVSCROLL);

    // Creates progress bar on window
    g_progressHWnd = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        (LPTSTR)NULL,
        WS_CHILD,
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
    SendMessage(g_progressHWnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // set range
    return TRUE;
}

BOOL UI::CreateCheckbox(HWND parentHWnd, RECT parentRect)
{
    g_checkboxHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        GetStringResource(IDS_STRING_LAUNCH_CHECKBOX).c_str(),  // text
        WS_TABSTOP | WS_CHILD | BS_AUTOCHECKBOX, // style
        parentRect.left + 50, // x coord
        parentRect.bottom - 60,  // y coord
        165,  // width
        35,  // height
        parentHWnd,  // parent
        (HMENU)IDC_LAUNCHCHECKBOX, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        NULL);

    //Set default checkbox state to checked
    SendMessage(g_checkboxHWnd, BM_SETCHECK, BST_CHECKED, 0);
    return TRUE;
}

BOOL UI::InstallButton(HWND parentHWnd, RECT parentRect) {
    LPVOID buttonPointer = nullptr;
    g_buttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"Button",
        GetStringResource(IDS_STRING_INSTALLTEXT).c_str(),  // text
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
        parentRect.right - 100 - 50, // x coord
        parentRect.bottom - 60,  // y coord
        120,  // width
        35,  // height
        parentHWnd,  // parent
        (HMENU)IDC_INSTALLBUTTON, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        buttonPointer); // pointer to button
    return TRUE;
}

BOOL UI::CreateCancelButton(HWND parentHWnd, RECT parentRect)
{
    LPVOID buttonPointer = nullptr;
    g_CancelbuttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        GetStringResource(IDS_STRING_UI_CANCEL).c_str(),  // text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
        parentRect.right - 100 - 50, // x coord
        parentRect.bottom - 60,  // y coord
        120,  // width
        35,  // height
        parentHWnd,  // parent
        (HMENU)IDC_CANCELBUTTON, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        buttonPointer); // pointer to button
    return TRUE;
}

BOOL UI::CreateLaunchButton(HWND parentHWnd, RECT parentRect, int xDiff, int yDiff)
{
    LPVOID buttonPointer = nullptr;
    g_LaunchbuttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        GetStringResource(IDS_STRING_LAUNCHBUTTON).c_str(),  // text
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
        parentRect.right - xDiff, // x coord
        parentRect.bottom - yDiff,  // y coord
        120,  // width
        35,  // height
        parentHWnd,  // parent
        (HMENU)IDC_LAUNCHBUTTON, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        buttonPointer); // pointer to button
    return TRUE;
}

BOOL UI::CreateDisplayPercentageText(HWND parentHWnd, RECT parentRect)
{
    int scrollHeight = GetSystemMetrics(SM_CYVSCROLL);

    g_percentageTextHWnd = CreateWindowEx(
        WS_EX_LEFT,
        L"Static",
        GetStringResource(IDS_STRING_INSTALLING_APP).c_str(),
        WS_CHILD ,
        parentRect.left + 50,
        parentRect.bottom - scrollHeight - 143,
        175,
        20,
        parentHWnd,
        (HMENU)IDC_STATICPERCENTCONTROL,
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        0);

    g_staticPercentText = CreateWindowEx(
        WS_EX_LEFT,
        L"Static",
        L"0%...",
        WS_CHILD,
        parentRect.left + 200,
        parentRect.bottom - scrollHeight - 143,
        175,
        20,
        parentHWnd,
        (HMENU)IDC_STATICPERCENTCONTROL,
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        0);
    return TRUE;
}

BOOL UI::CreateDisplayErrorText(HWND parentHWnd, RECT parentRect)
{
    g_staticErrorTextHWnd = CreateWindowEx(
        WS_EX_LEFT,
        L"Static",
        GetStringResource(IDS_STRING_ERROR_REASON_TEXT).c_str(),
        WS_CHILD,
        parentRect.left + 50,
        parentRect.bottom - 80,
        120,
        20,
        parentHWnd,
        (HMENU)IDC_STATICERRORCONTROL,
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        0);

    g_staticErrorDescHWnd = CreateWindowEx(
        WS_EX_LEFT,
        L"Static",
        L"",
        WS_CHILD | WS_BORDER,
        parentRect.left + 50,
        parentRect.bottom - 60,
        375,
        40,
        parentHWnd,
        (HMENU)IDC_STATICERRORCONTROL,
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        0);

    return TRUE;
}

BOOL UI::ChangeInstallButtonText(const std::wstring& newMessage)
{
    SendMessage(g_buttonHWnd, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(newMessage.c_str()));
    return ShowWindow(g_buttonHWnd, SW_SHOW);
}

BOOL UI::ChangeText(HWND parentHWnd, std::wstring displayName, std::wstring messageText, IStream* logoStream)
{
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(parentHWnd, &paint);

    Gdiplus::Graphics graphics(deviceContext);

    Gdiplus::RectF layoutRect(50, 50, g_width - 144, 100);
    Gdiplus::Font displayNameFont(L"Arial", 16);
    Gdiplus::Font messageFont(L"Arial", 10);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentNear);
    auto windowsTextColor = Gdiplus::Color();
    windowsTextColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOWTEXT));
    Gdiplus::SolidBrush textBrush(windowsTextColor);

    graphics.DrawString(displayName.c_str(), -1, &displayNameFont, layoutRect, &format, &textBrush);
    layoutRect.Y += 40;
    graphics.DrawString(messageText.c_str(), -1, &messageFont, layoutRect, &format, &textBrush);

    std::wstring capabilitiesHeading = GetStringResource(IDS_STRING_CAPABILITIES);
    layoutRect.Y += 40;
    graphics.DrawString(capabilitiesHeading.c_str(), -1, &messageFont, layoutRect, &format, &textBrush);

    layoutRect.Y += 17;
    for (std::wstring capability : m_capabilities)
    {
        std::wstring capabilityString = L"\x2022 " + GetStringResource(IDS_RUNFULLTRUST_CAPABILITY);
        graphics.DrawString(capabilityString.c_str(), -1, &messageFont, layoutRect, &format, &textBrush);
        layoutRect.Y += 20;
    }

    if (logoStream != nullptr)
    {
        // We shouldn't fail if the image can't be loaded, just don't show it.
        auto image = Gdiplus::Image::FromStream(logoStream, FALSE);
        if (image != nullptr)
        {
            Gdiplus::Status status = graphics.DrawImage(image, g_width - 200, 25);
            delete image;
        }
    }

    EndPaint(parentHWnd, &paint);

    return TRUE;
}

int UI::CreateInitWindow(HINSTANCE hInstance, int nCmdShow, const std::wstring& windowClass, const std::wstring& title)
{
    HWND hWnd = CreateWindow(
        const_cast<wchar_t*>(windowClass.c_str()),
        const_cast<wchar_t*>(title.c_str()),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_width,  // width of window
        g_height, // height of window
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

    SetHwnd(hWnd);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
    PreprocessRequest();
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

void UI::ButtonClicked()
{
    switch (m_type)
    {
    case InstallUIAdd:
    {
            m_msixResponse = m_packageManager->AddPackageAsync(m_path, DeploymentOptions::None, [this](const IMsixResponse & sender) {
                
            ShowWindow(g_percentageTextHWnd, SW_SHOW);
            UpdateDisplayPercent(sender.GetPercentage());
            SendMessage(g_progressHWnd, PBM_SETPOS, (WPARAM)sender.GetPercentage(), 0);
            switch (sender.GetStatus())
            {
                case InstallationStep::InstallationStepCompleted:
                {
                    SendInstallCompleteMsg();
                }
                break;
                case InstallationStep::InstallationStepError:
                {
                    DisplayError(sender.GetHResultTextCode());
                }
                break;
            }
        });
    }
    break;
    }
}

void UI::SendInstallCompleteMsg()
{
    SendMessage(GetHwnd(), WM_INSTALLCOMPLETE_MSG, NULL, NULL);
}

void UI::UpdateDisplayPercent(float displayPercent)
{
    std::wstringstream ss;
    ss << (int)displayPercent << "%...";
    SetWindowText(g_staticPercentText, ss.str().c_str());
    ShowWindow(g_staticPercentText, SW_HIDE);
    ShowWindow(g_staticPercentText, SW_SHOW);
}

void UI::DisplayError(HRESULT hr)
{
    g_installing = false;
    ShowWindow(g_percentageTextHWnd, SW_HIDE);
    ShowWindow(g_staticPercentText, SW_HIDE);
    ShowWindow(g_progressHWnd, SW_HIDE);
    ShowWindow(g_checkboxHWnd, SW_HIDE);
    ShowWindow(g_CancelbuttonHWnd, SW_HIDE);
    
    //Show Error Window
    ShowWindow(g_staticErrorTextHWnd, SW_SHOW);
    ShowWindow(g_staticErrorDescHWnd, SW_SHOW);
    std::wstringstream errorDescription;
    errorDescription << GetStringResource(IDS_STRING_ERROR_MSG) << std::hex << hr << ".";
    SetWindowText(g_staticErrorDescHWnd, errorDescription.str().c_str());
}

void UI::CloseUI()
{
    DestroyWindow(GetHwnd()); // close msix app installer
    SetEvent(m_closeUI);
}