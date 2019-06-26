#include "Windows10Redirector.hpp"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Management.Deployment.h>
#include <iostream>

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

HRESULT MsixCoreLib::Windows10Redirector::ConvertIStreamToPackagePath(IStream * packageStream, TCHAR tempFileName[])
{
    TCHAR tempPathBuffer[MAX_PATH];

    if (!GetTempPath(MAX_PATH, tempPathBuffer))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!GetTempFileName(tempPathBuffer, TEXT("MSIX"), 0, tempFileName))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::wcout << tempFileName << "\n";

    HANDLE tempFileHandle = INVALID_HANDLE_VALUE;
    tempFileHandle = CreateFile((LPTSTR)tempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (tempFileHandle == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

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
