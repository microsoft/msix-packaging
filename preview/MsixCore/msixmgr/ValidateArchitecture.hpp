#pragma once

#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"

namespace MsixCoreLib
{
    class ValidateArchitecture : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~ValidateArchitecture() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        ValidateArchitecture() {}
        ValidateArchitecture(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// This function returns true if the package architecture is compatible with the operating system
        bool IsArchitectureCompatibleWithOS();
    };
}