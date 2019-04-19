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

    /// the target device family name in the manifest
    std::wstring m_targetDeviceFamilyName;

    /// the parsed major version from the minversion manifest string
    std::wstring m_majorVersion;

    /// the parsed minor version from the minversion manifest string
    std::wstring m_minorVersion;

    /// the parsed build number from the minversion manifest string
    std::wstring m_buildNumber;

    ValidateTargetDeviceFamily() {}
    ValidateTargetDeviceFamily(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// This function parses the target device family fields from the app manifest
    ///
    HRESULT ParseTargetDeviceFamilyFromPackage();

    /// This function checks if the manifest version is compatible with the operating system
    ///
    bool IsManifestVersionCompatibleWithOS();
};
