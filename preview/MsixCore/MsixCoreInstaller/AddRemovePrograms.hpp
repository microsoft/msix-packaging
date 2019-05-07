#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"
#include "RegistryKey.hpp"

namespace MsixCoreLib
{
    /// Handles adding/removing the entry that allows an app to show up in AddRemovePrograms in the Control Panel (appwiz.cpl)
    class AddRemovePrograms : IPackageHandler
    {
    public:
        /// Creates a registry entry in the user's Uninstall key (in HKCU)
        /// This is read by the control panel's AddRemovePrograms to show packages that can be removed.
        HRESULT ExecuteForAddRequest();

        /// Creates a registry entry in the machine's Uninstall key (in HKLM)
        /// This is read by the control panel's AddRemovePrograms to show packages that can be removed.
        HRESULT ExecuteForAddForAllUsersRequest();

        /// Removes the registry entry from the user's Uninstall key (HKCU)
        HRESULT ExecuteForRemoveRequest();

        /// Removes the registry entry from the user's Uninstall key (HKLM)
        HRESULT ExecuteForRemoveForAllUsersRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~AddRemovePrograms() {}
    private:
        MsixRequest* m_msixRequest = nullptr;

        AddRemovePrograms() {}
        AddRemovePrograms(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// Writes the data to the already opened Uninstall key, which could be in HKLM or HKCU depending on whether it's for allusers or not
        HRESULT WriteUninstallKey(MsixCoreLib::RegistryKey & uninstallKey);
    };
}