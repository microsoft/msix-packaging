#pragma once

#include <netfw.h>
#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    /// the firewall rule structure
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

        /// Parses the manifest and fills in the m_firewallRules vector
        HRESULT ParseManifest();

        /// Adds the firewall rules from the manifest
        /// 
        /// @param firewallRule - the firewall rule object to add
        HRESULT AddFirewallRules(FirewallRule& firewallRule);

        /// Removes the firewall rule passed
        /// 
        /// @param firewallRuleName - the name of the firewall rule to remove
        HRESULT RemoveFirewallRules(BSTR firewallRuleName);

        HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);

        /// Returns the protocol as a 'short' type to be added to the firewall rule
        /// 
        /// @param key - the protocol from the manifest
        unsigned short ConvertToProtocol(PCWSTR key);

        /// Returns the profile as a 'NET_FW_PROFILE_TYPE2' type to be added to the firewall rule
        /// 
        /// @param key - the profile from the manifest
        NET_FW_PROFILE_TYPE2 ConvertToProfileType(PCWSTR key);
    };
}