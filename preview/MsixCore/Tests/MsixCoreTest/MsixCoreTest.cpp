// MsixCoreTest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MsixCoreTest.hpp"
#include "GeneralUtil.hpp"
#include <experimental/filesystem>

const std::wstring scribbleFileName = L"scribble.appx";
const std::wstring scribblePackageFullName = L"ScribbleOleDocumentSample_1.1.0.0_x86__8wekyb3d8bbwe";
const std::wstring windows10PackageRoot = L"C:\\Program Files\\Windowsapps";

MsixCoreTest::~MsixCoreTest()
{
    delete m_packageManager;
    m_packageManager = nullptr;
}

bool MsixCoreTest::SetupTest()
{
    // This assumes the machine already trusts the certificates that the packages are signed with,
    // and that the test packages are in the same directory as TE.exe and the dll.
    // Also if on windows 10, that the machine is sideloading enabled. 
    VERIFY_SUCCEEDED(MsixCoreLib_CreatePackageManager(&m_packageManager));
    VERIFY_SUCCEEDED(WEX::TestExecution::RuntimeParameters::TryGetValue(L"TestDeploymentDir", m_testDeploymentDir));
    return true;
}

bool MsixCoreTest::CleanupTest()
{
    delete m_packageManager;
    m_packageManager = nullptr;
    return true;
}

bool MsixCoreTest::SetupMethod()
{
    return true;
}

bool MsixCoreTest::CleanupMethod()
{
    return true;
}

void MsixCoreTest::InstallQueryAndRemoveWithLibTest()
{
    std::wstring packagePath = std::wstring(m_testDeploymentDir) + L"\\" + scribbleFileName;
    std::wstring expectedPackageFullName = scribblePackageFullName;
    VERIFY_SUCCEEDED(m_packageManager->AddPackage(packagePath, DeploymentOptions::None));

    std::unique_ptr<std::vector<std::shared_ptr<MsixCoreLib::IInstalledPackage>>> packages;
    VERIFY_SUCCEEDED(m_packageManager->FindPackages(packages));

    bool found = false;
    for (auto& package : *packages)
    {
        if (expectedPackageFullName == package->GetPackageFullName())
        {
            found = true;
        }
    }
    
    if (!MsixCoreLib::IsWindows10RS3OrLater())
    {
        VERIFY_IS_TRUE(found);
    }
    else
    {
        // FindPackages is NOT redirected on windows10, so we should not be able to find it using MsixCore because we MsixCore not actually install it.
        VERIFY_IS_FALSE(found);
        
        std::wstring windows10Location = windows10PackageRoot + L"\\" + expectedPackageFullName;
        VERIFY_IS_TRUE(std::experimental::filesystem::exists(windows10Location));
    }

    VERIFY_SUCCEEDED(m_packageManager->RemovePackage(expectedPackageFullName));
}

void MsixCoreTest::InstallWithLibAndGetProgressTest()
{
    std::wstring packagePath = std::wstring(m_testDeploymentDir) + L"\\" + scribbleFileName;
    std::wstring expectedPackageFullName = scribblePackageFullName;

    HANDLE completion = CreateEvent(nullptr, false, false, nullptr);
    UINT32 receivedCallbacks = 0;
    auto response = m_packageManager->AddPackageAsync(packagePath, DeploymentOptions::None, [&](const MsixCoreLib::IMsixResponse & sender)
    {
        receivedCallbacks++;
        if (sender.GetStatus() == MsixCoreLib::InstallationStep::InstallationStepCompleted)
        {
            SetEvent(completion);
        }
        WEX::Logging::Log::Comment(WEX::Common::String().Format(L"Received progress callback: %f", sender.GetPercentage()));
    });

    // Wait 10 seconds for completion.
    DWORD waitReturn = WaitForSingleObject(completion, 10000);
    VERIFY_IS_TRUE(receivedCallbacks > 1);
    VERIFY_ARE_EQUAL(waitReturn, WAIT_OBJECT_0);

    VERIFY_SUCCEEDED(m_packageManager->RemovePackage(expectedPackageFullName));
}

void MsixCoreTest::InstallIstreamPackageTest()
{
    std::wstring packagePath = std::wstring(m_testDeploymentDir) + L"\\" + scribbleFileName;
    std::wstring expectedPackageFullName = scribblePackageFullName;

    //Create package stream for scribble.appx
    IStream *packageStream = NULL;
    CreateStreamOnFileUTF16(packagePath.c_str(), /*forRead */ true, &packageStream);

    VERIFY_SUCCEEDED(m_packageManager->AddPackage(packageStream, DeploymentOptions::None));

    std::unique_ptr<std::vector<std::shared_ptr<MsixCoreLib::IInstalledPackage>>> packages;
    VERIFY_SUCCEEDED(m_packageManager->FindPackages(packages));

    bool found = false;
    for (auto& package : *packages)
    {
        if (expectedPackageFullName == package->GetPackageFullName())
        {
            found = true;
        }
    }

    if (!MsixCoreLib::IsWindows10RS3OrLater())
    {
        VERIFY_IS_TRUE(found);
    }
    else
    {
        // FindPackages is NOT redirected on windows10, so we should not be able to find it using MsixCore because we MsixCore not actually install it.
        VERIFY_IS_FALSE(found);

        std::wstring windows10Location = windows10PackageRoot + L"\\" + expectedPackageFullName;
        VERIFY_IS_TRUE(std::experimental::filesystem::exists(windows10Location));
    }

    VERIFY_SUCCEEDED(m_packageManager->RemovePackage(expectedPackageFullName));
}