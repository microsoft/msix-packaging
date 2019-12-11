#include <windows.h>

#include "PSFScriptExecuter.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "document.h"

using namespace MsixCoreLib;

const PCWSTR PSFScriptExecuter::HandlerName = L"PSFScriptExecuter";

HRESULT PSFScriptExecuter::ExecuteForAddRequest()
{
    if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
    {
        m_msixRequest->GetMsixResponse()->SetErrorStatus(HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT), L"User cancelled installation.");
        return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
    }

    // Read script parameters from PSF config and update executionInfo
    RETURN_IF_FAILED(m_msixRequest->GetPackageInfo()->ProcessPSFIfNecessary());

    std::wstring scriptName = m_msixRequest->GetPackageInfo()->GetScriptSettings()->scriptPath;
    if (scriptName.length() == 0)
    {
        return S_OK;
    }

    std::wstring workingDirectory = m_msixRequest->GetPackageInfo()->GetExecutionInfo()->workingDirectory;

    std::wstring scriptPath = workingDirectory + L"\\" + scriptName;
    std::wstring psArguments = L"-file \"" + scriptPath + L"\"";

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Executing PSF script",
        TraceLoggingValue(scriptPath.c_str(), "ScriptPath"),
        TraceLoggingValue(psArguments.c_str(), "Arguments"));

    bool showWindow = m_msixRequest->GetPackageInfo()->GetScriptSettings()->showWindow;
    INT showCmd = (showWindow) ? SW_SHOW : SW_HIDE;

    SHELLEXECUTEINFOW shellExecuteInfo = {};
    shellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellExecuteInfo.lpFile = L"powershell.exe";
    shellExecuteInfo.lpParameters = psArguments.c_str();
    shellExecuteInfo.nShow = showCmd;

    // Run the script, and wait for it to finish if desired
    ShellExecuteExW(&shellExecuteInfo);
    bool waitForScriptToFinish = m_msixRequest->GetPackageInfo()->GetScriptSettings()->waitForScriptToFinish;
    if (waitForScriptToFinish)
    {
        WaitForSingleObject(shellExecuteInfo.hProcess, INFINITE);
        DWORD exitCode = 0;
        if (!GetExitCodeProcess(shellExecuteInfo.hProcess, &exitCode))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to get exit code of PSF script");
        }

        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "PSF script exit code",
            TraceLoggingValue(exitCode, "Exit code"));
    }
    CloseHandle(shellExecuteInfo.hProcess);
    
    return S_OK;
}

HRESULT PSFScriptExecuter::ExecuteForRemoveRequest()
{
    return S_OK;
}

HRESULT PSFScriptExecuter::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PSFScriptExecuter> localInstance(new PSFScriptExecuter(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
