#pragma once

#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"
#include "RegistryKey.hpp"

namespace MsixCoreLib
{
    /// The database is a registry tree that indicates which user has which packages installed.
    /// After a successful add or remove, this handler updates the registry to reflect the result of the operation.
    class UpdateDatabase : IPackageHandler
    {
    public:

        /// This updates the database to indicate that the package is now installed
        HRESULT ExecuteForAddRequest();

        /// This updates the database to indicate the package is no longer installed
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~UpdateDatabase() {}

    private:
        MsixRequest * m_msixRequest = nullptr;

        UpdateDatabase() {}
        UpdateDatabase(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    };
}