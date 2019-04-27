#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

namespace MsixCoreLib
{
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
        HRESULT ParseTargetDeviceFamilyFromPackage();

        /// This function returns true if the target device family name is compatible with the operating system
        /// Desktop OS is compatible with MsixCore.Desktop and MsixCore.Server, Server OS is compatible with MsixCore.Server
        bool IsTargetDeviceFamilyNameCompatibleWithOS();

        /// This function returns true if the manifest version is compatible with the operating system
        bool IsManifestVersionCompatibleWithOS();

        /// This function returns true for Desktop OS
        ///
        /// @return true in case of desktop OS
        bool IsWindowsProductTypeDesktop();

        /// This function returns true for Server OS
        ///
        /// @return true in case of Server OS
        bool IsWindowsProductTypeServer();
    };
}