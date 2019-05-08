#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    /// Handles adding/removing the entry that allows an app to show up in AddRemovePrograms in the Control Panel (appwiz.cpl)
    class AddRemovePrograms : IPackageHandler
    {
    public:
        /// Creates a registry entry in the Uninstall key.
        /// This is read by the control panel's AddRemovePrograms to show packages that can be removed.
        HRESULT ExecuteForAddRequest();

        /// Removes the registry entry.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~AddRemovePrograms() {}
    private:
        MsixRequest* m_msixRequest = nullptr;

        AddRemovePrograms() {}
        AddRemovePrograms(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    };
}