#pragma once

#include <netfw.h>
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class FirewallRules : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        /// Removes all the registry keys written during the add.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~FirewallRules() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        FirewallRules() {}
        FirewallRules(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        HRESULT ParseManifest();

        HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);

        unsigned short ConvertToProtocol(PCWSTR key);

        NET_FW_PROFILE_TYPE2 ConvertToProfileType(PCWSTR key);
    };
}