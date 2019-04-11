#pragma once
/// Install UI Header
/// UI Functions
#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "resource.h"

/// Child window identifiers
#define IDC_LAUNCHCHECKBOX 101
#define IDC_INSTALLBUTTON 102
#define IDC_CANCELBUTTON 103
#define IDC_LAUNCHBUTTON 104
#define WM_INSTALLCOMPLETE_MSG (WM_APP+1)

/// Global variables
static HWND g_buttonHWnd = NULL;
static HWND g_checkboxHWnd = NULL;
static HWND g_progressHWnd = NULL;
static HWND g_CancelbuttonHWnd = NULL;
static HWND g_LaunchbuttonHWnd = NULL;
static bool g_launchCheckBoxState = true; /// launch checkbox is checked by default
static bool g_installing = false; /// variable used to indicate that app installation is in progress

class UI
{
public:
    HRESULT ShowUI();
    HRESULT LaunchInstalledApp();
    void ConfirmAppCancel(HWND parentHWnd);

    static HRESULT Make(_In_ MsixRequest* msixRequest, _Out_ UI** instance);
    ~UI() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    //Parent Window Hwnd
    HWND hWnd = NULL; 

    /// Default prefix text on UI while prompting for app installation is 'Install'
    std::wstring m_installOrUpdateText = GetStringResource(IDS_STRING_INSTALLTEXT);

    /// Message to prompt user while cancelling app installation
    std::wstring m_cancelPopUpMessage = GetStringResource(IDS_STRING_CANCEL_INSTALLPOPUP);

    /// Popup message title while cancelling app installation
    std::wstring m_cancelPopUpTitle = GetStringResource(IDS_STRING_CANCEL_POPUP_TITLE);
    std::wstring m_displayName = L"";
    std::wstring m_publisherCommonName = L"";
    ComPtr<IStream> m_logoStream;
    std::wstring m_version = L"";
    int m_numberOfFiles = 0;
    HRESULT m_loadingPackageInfoCode = 0;
    HANDLE m_buttonClickedEvent;

    UI() {}
    UI(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) 
	{
		m_buttonClickedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
    
    HRESULT ParseInfoFromPackage();

    /// This function sets the parent window hwnd after create window
    ///
    /// @param hWnd - the HWND of the parent window
    void SetHwnd(HWND hwnd)
    {
        hWnd = hwnd;
    }

    /// This function returns the parent window hwnd
    ///
    /// @return hWnd: parent window hwnd
    HWND GetHwnd()
    {
        return hWnd;
    }

public:

    /// This function compiles the information displayed on the UI when the user selects an msix
    ///
    /// @param hWnd - the HWND of the window to draw controls
    /// @param windowRect - the size of the window
    HRESULT DrawPackageInfo(HWND hWnd, RECT windowRect);

    /// Creates the initial installation UI window
    ///
    /// @param windowClass - the class text of the window
    /// @param windowTitle - the window title
    int CreateInitWindow(HINSTANCE hInstance, int nCmdShow, const std::wstring& windowClass, const std::wstring& title);
    void LoadInfo();
    int GetNumberOfFiles() { return m_numberOfFiles; }
    void SetButtonClicked() { SetEvent(m_buttonClickedEvent); }

    /// Creates the progress bar
    ///
    /// @param parentHWnd - the HWND of the window to add the progress bar to
    /// @param parentRect - the dimmensions of the parent window
    /// count: the number of objects to be iterated through in the progress bar
    BOOL CreateProgressBar(HWND parentHWnd, RECT parentRect, int count);

    /// Create the lower right install button
    /// 
    /// @param parentHWnd - the HWND of the window to add the button to
    /// @param parentRect - the specs of the parent window
    BOOL InstallButton(HWND parentHWnd, RECT parentRect);

    /// Create the launch when ready checkbox on the bottom left
    /// 
    /// @param parentHWnd - the HWND of the window to add the checkbox to
    /// @param parentRect - the specs of the parent window
    BOOL CreateCheckbox(HWND parentHWnd, RECT parentRect);

    /// Create the cancel button on the bottom right corner when user clicks on install
    /// 
    /// @param parentHWnd - the HWND of the window to add the checkbox to
    /// @param parentRect - the specs of the parent window
    BOOL CreateCancelButton(HWND parentHWnd, RECT parentRect);

    /// Create the launch button on the botton right after app has been installed
    /// 
    /// @param parentHWnd - the HWND of the window to add the checkbox to
    /// @param parentRect - the specs of the parent window
    /// @param xDiff - the x coordinate difference to create the button from the parent window
    /// @param yDiff - the y coordinate difference to create the button from the parent window
    BOOL CreateLaunchButton(HWND parentHWnd, RECT parentRect, int xDiff, int yDiff);

    /// Changes the text of the lower right 'Install' button
    /// Changes text to 'Update' in case of an update, changes text to 'Reinstall' in case app is already installed on the machine
    ///
    /// @param newMessage - the message to change the button to
    BOOL ChangeInstallButtonText(const std::wstring& newMessage);

    /// Change the text of the installation window based on the given input
    ///
    /// @param parentHWnd - the HWND of the window to be changed
    /// @param windowText - the text to change the window to
    BOOL ChangeText(HWND parentHWnd, std::wstring displayText, std::wstring  messageText, IStream* logoStream = nullptr);

    /// Increment the progress bar one tick based on preset tick
    void UpdateProgressBar();

    /// Sends the WM_INSTALLCOMPLETE_MSG message to the main window when app installation is complete
    void SendInstallCompleteMsg();

    // The add operation could be an update if V1 version of the package is already installed. Show appropriate UI with respect to operation type
    /// The add operation could be an update if V1 version of the package is already installed on the machine
    /// This method checks the same and sets the button and install screen UI text to 'Update'
    ///
    void PreprocessRequest();

};

class CreateAndShowUI : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~CreateAndShowUI() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    CreateAndShowUI() {}
    CreateAndShowUI(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}
};
