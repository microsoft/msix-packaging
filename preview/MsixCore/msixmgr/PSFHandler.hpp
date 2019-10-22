#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class PSFHandler : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~PSFHandler() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        PSFHandler() {}
        PSFHandler(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    };
}