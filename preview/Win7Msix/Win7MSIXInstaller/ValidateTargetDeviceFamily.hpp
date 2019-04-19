#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class ValidateTargetDeviceFamily : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ValidateTargetDeviceFamily() {}
private:
    MsixRequest * m_msixRequest = nullptr;

    std::wstring m_targetDeviceFamilyName;
    std::wstring m_majorVersion;
    std::wstring m_minorVersion;
    std::wstring m_buildNumber;

    ValidateTargetDeviceFamily() {}
    ValidateTargetDeviceFamily(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    HRESULT ParseTargetDeviceFamilyFromPackage();
    bool IsManifestVersionCompatibleWithOS();
};
