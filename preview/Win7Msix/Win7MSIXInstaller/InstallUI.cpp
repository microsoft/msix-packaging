    // UI Functions

#include "InstallUI.hpp"
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <sys/types.h>
#include <thread>

#include <algorithm>
#include <sstream>
#include <iostream>
#include "resource.h"

// MSIXWindows.hpp define NOMINMAX because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. Use std namespace *BEFORE* including it.
using namespace std;
#include <GdiPlus.h>

static const int g_width = 500;  // width of window
static const int g_heigth = 400; // height of window

const PCWSTR CreateAndShowUI::HandlerName = L"UI";

//
// Gets the stream of a file.
//
// Parameters:
//   package - The package reader for the app package.
//   name - Name of the file.
//   stream - The stream for the file.
//
HRESULT GetStreamFromFile(IAppxPackageReader* package, LPCWCHAR name, IStream** stream)
{
    *stream = nullptr;

    ComPtr<IAppxFilesEnumerator> files;
    RETURN_IF_FAILED(package->GetPayloadFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));
        Text<WCHAR> fileName;
        file->GetName(&fileName);
        if (wcscmp(fileName.Get(), name) == 0)
        {
            RETURN_IF_FAILED(file->GetStream(stream));
            return S_OK;
        }
        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
    }
    return S_OK;
}

HRESULT UI::DrawPackageInfo(HWND hWnd, RECT windowRect)
{
    if (SUCCEEDED(m_loadingPackageInfoCode))
    {
        auto displayText = L"Install " + m_displayName + L"?";
        auto messageText = L"Publisher: " + m_publisherCommonName + L"\nVersion: " + m_version;
        ChangeText(hWnd, displayText, messageText, m_logoStream.Get());
        ChangeText(hWnd, GetStringResource(IDS_STRING_UI_INSTALL_COMPLETE), GetStringResource(IDS_STRING_UI_COMPLETION_MESSAGE));
    }
    else
    {
        std::wstringstream wstringstream;
        wstringstream << L"Failed getting package information with: 0x" << std::hex << m_loadingPackageInfoCode;
        auto g_messageText = wstringstream.str();
        ChangeText(hWnd, L"Loading Package failed", g_messageText);
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
        ui->LaunchButton(hWnd, windowRect);
        ui->CreateCheckbox(hWnd, windowRect);
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
                g_installed = true;
                DestroyWindow(g_buttonHWnd);
                ui->CreateCancelButton(hWnd, windowRect);
                UpdateWindow(hWnd);
                if (ui != NULL)
                {
                    ui->CreateProgressBar(hWnd, windowRect, ui->GetNumberOfFiles());
                }
                ShowWindow(g_progressHWnd, SW_SHOW); //Show progress bar only when install is clicked
                if (ui != NULL)
                {
                    ui->SetButtonClicked();
                }   
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
                ui->ConfirmAppCancel();
                break;
            }
            case IDC_LAUNCHBUTTON:
            {
                ui->LaunchInstalledApp();
                break;
            }
        }
        break;
    case WM_INSTALLCOMPLETE_MSG:
    {
        DestroyWindow(g_CancelbuttonHWnd);
        ui->CreateLaunchButton(hWnd, windowRect);
        UpdateWindow(hWnd);
        ShowWindow(g_progressHWnd, SW_HIDE); //hide progress bar
        ShowWindow(g_checkboxHWnd, SW_HIDE); //hide launch check box
        if (g_launchCheckBoxState)
        {
            ui->LaunchInstalledApp(); // launch app
            DestroyWindow(hWnd); // close msix app installer
        }
        else
        {
            //wait for user to click launch button or close the window
            while (true)
            {
                switch (MsgWaitForMultipleObjects(0, NULL, FALSE, INFINITE, QS_ALLINPUT))
                {
                case WAIT_OBJECT_0:
                    MSG msg;
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    break;
                }
            }
        }
        break;
    }
	case WM_CTLCOLORSTATIC:
	{
        switch (::GetDlgCtrlID((HWND)lParam))
	    {
            case IDC_LAUNCHCHECKBOX:
            {
                HBRUSH hbr = (HBRUSH)DefWindowProc(hWnd, message, wParam, lParam);
                ::DeleteObject(hbr);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (LRESULT)::GetStockObject(NULL_BRUSH);
            }
        }
        break;
    }
    case WM_CLOSE:
        if (g_installed)
        {
            ui->ConfirmAppCancel();
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

void UI::ConfirmAppCancel()
{
    const int cancelResult = MessageBox(hWnd, L"Are you sure you want to cancel the install?", L"Cancel App install", MB_YESNO);
    switch (cancelResult)
    {
    case IDYES:
        m_msixRequest->CancelRequest();
        break;
    case IDNO:
        break;
    }
}

HRESULT UI::LaunchInstalledApp()
{
    PackageInfo* packageInfo = m_msixRequest->GetPackageInfo();
    std::wstring resolvedExecutableFullPath = packageInfo->GetExecutableFilePath();
    //check for error while launching app here
    ShellExecute(NULL, NULL, resolvedExecutableFullPath.c_str(), NULL, NULL, SW_SHOW);
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

void UI::LoadInfo()
{
    m_loadingPackageInfoCode = ParseInfoFromPackage();
}

HRESULT UI::ParseInfoFromPackage() 
{
    PackageInfo* packageInfo = m_msixRequest->GetPackageInfo();

    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(packageInfo->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    // Obtain the Display Name and Logo
    ComPtr<IMsixElementEnumerator> veElementEnum;
    RETURN_IF_FAILED(element->GetElements(
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']",
        &veElementEnum));

    // Obtain publisher name
    auto wpublisher = std::wstring(packageInfo->GetPublisher());
    m_publisherCommonName = wpublisher.substr(wpublisher.find_first_of(L"=") + 1,
        wpublisher.find_first_of(L",") - wpublisher.find_first_of(L"=") - 1);

    // Obtain version number
    ConvertVersionToString(packageInfo->GetVersion());

    //Obtain the number of files
    m_numberOfFiles = packageInfo->GetNumberOfPayloadFiles();

    // Obtain logo
    BOOL hc = FALSE;
    RETURN_IF_FAILED(veElementEnum->GetHasCurrent(&hc));
    if (hc)
    {
        ComPtr<IMsixElement> visualElementsElement;
        Text<WCHAR> displayNameValue;
        RETURN_IF_FAILED(veElementEnum->GetCurrent(&visualElementsElement));
        RETURN_IF_FAILED(visualElementsElement->GetAttributeValue(L"DisplayName", &displayNameValue));
        m_displayName = std::wstring(displayNameValue.Get());
        Text<WCHAR> logo;
        RETURN_IF_FAILED(visualElementsElement->GetAttributeValue(L"Square150x150Logo", &logo));
        RETURN_IF_FAILED(GetStreamFromFile(packageInfo->GetPackageReader(), logo.Get(), &m_logoStream));
    }
    return S_OK;
}

HRESULT UI::ShowUI()
{
    std::thread thread(StartUIThread, this);
    thread.detach();

    DWORD waitResult = WaitForSingleObject(m_buttonClickedEvent, INFINITE);
    
    return S_OK;
}

HRESULT CreateAndShowUI::ExecuteForAddRequest()
{
    if (m_msixRequest->IsQuietUX())
    {
        return S_OK;
    }

    AutoPtr<UI> ui;
    RETURN_IF_FAILED(UI::Make(m_msixRequest, &ui));
    ui->LoadInfo();

    m_msixRequest->SetUI(ui.Detach());
    RETURN_IF_FAILED(m_msixRequest->GetUI()->ShowUI());

    return S_OK;
}

HRESULT CreateAndShowUI::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<CreateAndShowUI> localInstance(new CreateAndShowUI(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}

HRESULT UI::Make(MsixRequest * msixRequest, UI ** instance)
{
    std::unique_ptr<UI> localInstance(new UI(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}

BOOL UI::CreateProgressBar(HWND parentHWnd, RECT parentRect, int count)
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
    SendMessage(g_progressHWnd, PBM_SETRANGE, 0, MAKELPARAM(0, count)); // set range
    SendMessage(g_progressHWnd, PBM_SETSTEP, (WPARAM)1, 0); // set increment
    return TRUE;
}

BOOL UI::CreateCheckbox(HWND parentHWnd, RECT parentRect)
{
    g_checkboxHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        L"Launch when ready",  // text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // style
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

BOOL UI::LaunchButton(HWND parentHWnd, RECT parentRect) {
    LPVOID buttonPointer = nullptr;
    g_buttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        L"Install",  // text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
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
		L"Cancel",  // text
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

BOOL UI::CreateLaunchButton(HWND parentHWnd, RECT parentRect) 
{
    LPVOID buttonPointer = nullptr;
    g_LaunchbuttonHWnd = CreateWindowEx(
        WS_EX_LEFT, // extended window style
        L"BUTTON",
        L"Launch",  // text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, // style
        parentRect.right - 100 - 50, // x coord
        parentRect.bottom - 60,  // y coord
        120,  // width
        35,  // height
        parentHWnd,  // parent
        (HMENU)IDC_LAUNCHBUTTON, // menu
        reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parentHWnd, GWLP_HINSTANCE)),
        buttonPointer); // pointer to button
    return TRUE;
}

BOOL UI::ChangeButtonText(const std::wstring& newMessage)
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
    hWnd = CreateWindow(
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

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this); 
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

void UI::UpdateProgressBar()
{
    SendMessage(g_progressHWnd, PBM_STEPIT, 0, 0);
}

void UI::SendInstallCompleteMsg()
{
    SendMessage(hWnd, WM_INSTALLCOMPLETE_MSG, NULL, NULL);
}
