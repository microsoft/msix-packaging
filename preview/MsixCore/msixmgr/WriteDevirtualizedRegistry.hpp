#pragma once

#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class WriteDevirtualizedRegistry : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    private:
        MsixRequest * m_msixRequest = nullptr;

        WriteDevirtualizedRegistry() {}
        WriteDevirtualizedRegistry(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}
    };
}