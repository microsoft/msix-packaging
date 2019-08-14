#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    /// the AppExecutionAlias structure
    /*struct AppExecutionAliasObject
    {
        std::wstring key;
        std::wstring executableFullPath;


    };*/

    class AppExecutionAlias : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~AppExecutionAlias() {}
    private:
        MsixRequest * m_msixRequest = nullptr;
        std::vector<std::wstring> m_appExecutionAliases;

        AppExecutionAlias() {}
        AppExecutionAlias(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        HRESULT ParseManifest();

        HRESULT ProcessAliasForAdd(std::wstring & aliasName);

    };
}