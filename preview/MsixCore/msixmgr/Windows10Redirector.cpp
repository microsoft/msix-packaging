#include "Windows10Redirector.hpp"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Management.Deployment.h>
#include "MsixTraceLoggingProvider.hpp"
#include <sddl.h>
#include <AclAPI.h>

HRESULT MsixCoreLib::Windows10Redirector::AddPackageWithProgress(const std::wstring & packageFilePath, std::shared_ptr<MsixResponse>& msixResponse)
{
    winrt::init_apartment();
    
    winrt::Windows::Foundation::Uri packageUri{ packageFilePath };
    winrt::Windows::Management::Deployment::PackageManager packageManager;

    auto deploymentOperation{ packageManager.AddPackageAsync(packageUri, nullptr, winrt::Windows::Management::Deployment::DeploymentOptions::None) };
    deploymentOperation.Progress([&](auto const& /*sender */, winrt::Windows::Management::Deployment::DeploymentProgress progress)
    {
        if (progress.percentage == 100)
        {
            msixResponse->Update(InstallationStep::InstallationStepCompleted, (float)progress.percentage);
        }
        else
        {
            msixResponse->Update(InstallationStep::InstallationStepExtraction, (float)progress.percentage);
        }
    });

    deploymentOperation.get();
    if (deploymentOperation.Status() == winrt::Windows::Foundation::AsyncStatus::Error)
    {
        auto deploymentResult{ deploymentOperation.GetResults() };
        msixResponse->SetErrorStatus(deploymentOperation.ErrorCode(), deploymentResult.ErrorText().c_str());
    }
    return S_OK;
}

HRESULT MsixCoreLib::Windows10Redirector::AddPackage(const std::wstring & packageFilePath)
{
    winrt::init_apartment();

    winrt::Windows::Foundation::Uri packageUri{ packageFilePath };
    winrt::Windows::Management::Deployment::PackageManager packageManager;

    auto deploymentOperation{ packageManager.AddPackageAsync(packageUri, nullptr, winrt::Windows::Management::Deployment::DeploymentOptions::None) };
    deploymentOperation.get();
    if (deploymentOperation.Status() == winrt::Windows::Foundation::AsyncStatus::Error)
    {
        auto deploymentResult{ deploymentOperation.GetResults() };
        return deploymentOperation.ErrorCode();
    }

    return S_OK;
}

HRESULT MsixCoreLib::Windows10Redirector::RemovePackage(const std::wstring & packageFullName)
{
    winrt::init_apartment();

    winrt::Windows::Management::Deployment::PackageManager packageManager;

    auto deploymentOperation{ packageManager.RemovePackageAsync(packageFullName) };
    deploymentOperation.get();

    return S_OK;
}

HRESULT SetAclOnFile(HANDLE file)
{
    struct LocalAlloc_delete
    {
        LocalAlloc_delete() { }
        void operator()(SECURITY_DESCRIPTOR* p) throw() { LocalFree(p); }
    };

    // Defend against lower privileged users-- allow users read execute, but disallow write
    PSECURITY_DESCRIPTOR psd;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(L"O:BAD:PAI(A;ID;FA;;;SY)(A;ID;FA;;;BA)(A;ID;FRFX;;;BU)", SDDL_REVISION_1, &psd, nullptr))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    std::unique_ptr<SECURITY_DESCRIPTOR, LocalAlloc_delete> securityDescriptor(reinterpret_cast<SECURITY_DESCRIPTOR*>(psd));

    PACL dacl;
    BOOL daclPresent = FALSE;
    BOOL daclDefaulted = FALSE;
    if (!GetSecurityDescriptorDacl(psd, &daclPresent, &dacl, &daclDefaulted))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD error = SetSecurityInfo(file, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, nullptr, nullptr, dacl, nullptr);
    if (error != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(error);
    }

    return S_OK;
}

HRESULT MsixCoreLib::Windows10Redirector::ConvertIStreamToPackagePath(IStream * packageStream, TCHAR tempPackagePath[])
{
    TCHAR tempPathBuffer[MAX_PATH];

    if (!GetTempPath(MAX_PATH, tempPathBuffer))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!GetTempFileName(tempPathBuffer, TEXT("MSIX"), 0, tempPackagePath))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HANDLE tempFileHandle = INVALID_HANDLE_VALUE;
    tempFileHandle = CreateFile((LPTSTR)tempPackagePath, GENERIC_READ | GENERIC_WRITE | WRITE_DAC, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (tempFileHandle == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    RETURN_IF_FAILED(SetAclOnFile(tempFileHandle));

    LARGE_INTEGER start = { 0 };
    ULARGE_INTEGER pos = { 0 };
    RETURN_IF_FAILED(packageStream->Seek(start, STREAM_SEEK_SET, &pos));

    ULONG dwBytesRead = 0;
    DWORD dwBytesWritten = 0;

    const int numOfBytes = 2048;
    BYTE buffer[numOfBytes];
    HRESULT hr;

    while ((hr = packageStream->Read(buffer, numOfBytes, &dwBytesRead)) == S_OK)
    {
        if (dwBytesRead > 0)
        {
            if (!WriteFile(tempFileHandle, buffer, dwBytesRead, &dwBytesWritten, NULL))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            break;
        }
    }

    if (!CloseHandle(tempFileHandle))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}
