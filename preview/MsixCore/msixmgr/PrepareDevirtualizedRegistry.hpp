#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class PrepareDevirtualizedRegistry : IPackageHandler
    {
    public:
        /// Mount the registry.dat file and set it in the msixRequest so future handlers can access the information within
        HRESULT ExecuteForAddRequest();

        /// Mount the registry.dat file and set it in the msixRequest so future handlers can access the information within
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~PrepareDevirtualizedRegistry() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        PrepareDevirtualizedRegistry() {}
        PrepareDevirtualizedRegistry(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// Mount the registry.dat file and set it in the msixRequest so future handlers can access the information within
        HRESULT ExtractRegistry();
    };
}