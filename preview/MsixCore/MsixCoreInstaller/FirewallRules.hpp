#pragma once

#include <netfw.h>
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    struct FirewallRule
    {
        std::wstring direction;
        std::wstring protocol;
        std::wstring profile;
        std::wstring localPortMin;
        std::wstring localPortMax;
        std::wstring remotePortMin;
        std::wstring remotePortMax;
    };

    class FirewallRules : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~FirewallRules() {}
    private:
        MsixRequest * m_msixRequest = nullptr;
        std::vector<FirewallRule> m_firewallRules;

        FirewallRules() {}
        FirewallRules(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        HRESULT ParseManifest();

        HRESULT AddFirewallRules(FirewallRule& firewallRule);

        HRESULT RemoveFirewallRules(BSTR firewallRuleName);

        HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);

        unsigned short ConvertToProtocol(PCWSTR key);

        NET_FW_PROFILE_TYPE2 ConvertToProfileType(PCWSTR key);
    };
}