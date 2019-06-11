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
