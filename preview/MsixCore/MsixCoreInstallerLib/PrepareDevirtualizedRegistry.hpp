#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class PrepareDevirtualizedRegistry : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        /// Removes all the registry keys written during the add.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~PrepareDevirtualizedRegistry() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        PrepareDevirtualizedRegistry() {}
        PrepareDevirtualizedRegistry(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// Extracts or removes the registry information contained inside Registry.dat
        ///
        /// @param remove - if true, removes registry information. if false, adds registry information
        HRESULT ExtractRegistry(bool remove);
    };
}