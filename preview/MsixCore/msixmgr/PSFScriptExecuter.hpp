#pragma once

#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    /// Handles executing the PSF (Package Support Framework) script from the appxmanifest/config.json
    /// More information about PSF can be found https://github.com/microsoft/MSIX-PackageSupportFramework
    class PSFScriptExecuter : IPackageHandler
    {
    public:
        /// Runs the runonce script associated with the PSF
        HRESULT ExecuteForAddRequest();

        /// Nothing to do on a remove - no notion of a uninstall script in PSF yet.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~PSFScriptExecuter() {}
    private:
        MsixRequest* m_msixRequest = nullptr;

        PSFScriptExecuter() {}
        PSFScriptExecuter(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}
    };
}
