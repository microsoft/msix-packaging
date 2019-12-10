#include <windows.h>
#include <stdio.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <netfw.h>

#include "FirewallRules.hpp"
#include "../GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "../MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

using namespace MsixCoreLib;

const PCWSTR FirewallRules::HandlerName = L"FirewallRules";

HRESULT FirewallRules::ExecuteForAddRequest()
{
    for (auto firewallRule = m_firewallRules.begin(); firewallRule != m_firewallRules.end(); ++firewallRule)
    {
        RETURN_IF_FAILED(AddFirewallRules(*firewallRule));
    }
    return S_OK;
}

HRESULT FirewallRules::ExecuteForRemoveRequest()
{
    for (auto firewallRule = m_firewallRules.begin(); firewallRule != m_firewallRules.end(); ++firewallRule)
    {
        //Create rule name
        std::wstring packageDisplayName = m_msixRequest->GetPackageInfo()->GetDisplayName();
        std::wstring ruleNameString = packageDisplayName.data();
        ruleNameString.append(L" (");
        ruleNameString.append(firewallRule->protocol);
        ruleNameString.append(L"-");
        ruleNameString.append(firewallRule->direction);
        ruleNameString.append(L")");

        BSTR ruleName = SysAllocString(ruleNameString.data());

        RETURN_IF_FAILED(RemoveFirewallRules(ruleName));
    }  
    return S_OK;
}

HRESULT FirewallRules::RemoveFirewallRules(BSTR firewallRuleName)
{
    ComPtr<INetFwPolicy2> pNetFwPolicy2;
    ComPtr<INetFwRules> pFwRules;
    ComPtr<INetFwRule> pFwRule;

    // Retrieve INetFwPolicy2
    RETURN_IF_FAILED(WFCOMInitialize(&pNetFwPolicy2));

    // Retrieve INetFwRules
    RETURN_IF_FAILED(pNetFwPolicy2->get_Rules(&pFwRules));

    // Create a new Firewall Rule object.
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(NetFwRule), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), (void**)&pFwRule));

    RETURN_IF_FAILED(pFwRules->Remove(firewallRuleName));

    return S_OK;
}

HRESULT FirewallRules::ParseManifest()
{
    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(m_msixRequest->GetPackageInfo()->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    ComPtr<IMsixElementEnumerator> firewallRuleEnum;
    RETURN_IF_FAILED(element->GetElements(firewallExtensionQuery.c_str(), &firewallRuleEnum));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(firewallRuleEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        FirewallRule firewallRule;
        ComPtr<IMsixElement> ruleElement;
        RETURN_IF_FAILED(firewallRuleEnum->GetCurrent(&ruleElement));

        Text<wchar_t> direction;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(firewallRuleDirectionAttribute.c_str(), &direction));
        firewallRule.direction = direction.Get();

        Text<wchar_t> protocol;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(protocolAttribute.c_str(), &protocol));
        firewallRule.protocol = protocol.Get();

        Text<wchar_t> profile;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(profileAttribute.c_str(), &profile));
        firewallRule.profile = profile.Get();

        Text<wchar_t> localPortMin, localPortMax, remotePortMin, remotePortMax;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(localPortMinAttribute.c_str(), &localPortMin));
        if (localPortMin.Get() != nullptr)
        {
            firewallRule.localPortMin = localPortMin.Get();
        }

        RETURN_IF_FAILED(ruleElement->GetAttributeValue(localPortMaxAttribute.c_str(), &localPortMax));
        if (localPortMax.Get() != nullptr)
        {
            firewallRule.localPortMax = localPortMax.Get();
        }

        RETURN_IF_FAILED(ruleElement->GetAttributeValue(remotePortMinAttribute.c_str(), &remotePortMin));
        if (remotePortMin.Get() != nullptr)
        {
            firewallRule.remotePortMin = remotePortMin.Get();
        }

        RETURN_IF_FAILED(ruleElement->GetAttributeValue(remotePortMaxAttribute.c_str(), &remotePortMax));
        if (remotePortMax.Get() != nullptr)
        {
            firewallRule.remotePortMax = remotePortMax.Get();
        }

        m_firewallRules.push_back(firewallRule);

        RETURN_IF_FAILED(firewallRuleEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT FirewallRules::AddFirewallRules(FirewallRule& firewallRule)
{
    ComPtr<INetFwPolicy2> pNetFwPolicy2;
    ComPtr<INetFwRules> pFwRules;
    ComPtr<INetFwRule> pFwRule;

    // Retrieve INetFwPolicy2
    RETURN_IF_FAILED(WFCOMInitialize(&pNetFwPolicy2));

    // Retrieve INetFwRules
    RETURN_IF_FAILED(pNetFwPolicy2->get_Rules(&pFwRules));

    // Create a new Firewall Rule object.
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(NetFwRule), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), (void**)&pFwRule));

    // Create rule name
    std::wstring packageDisplayName = m_msixRequest->GetPackageInfo()->GetDisplayName();

    std::wstring ruleNameString = packageDisplayName.data();
    ruleNameString.append(L" (");
    ruleNameString.append(firewallRule.protocol.c_str());
    ruleNameString.append(L"-");
    ruleNameString.append(firewallRule.direction.c_str());
    ruleNameString.append(L")");

    BSTR ruleName = SysAllocString(ruleNameString.data());
    BSTR ruleDescription = SysAllocString(packageDisplayName.data());
    RETURN_IF_FAILED(pFwRule->put_Name(ruleName));
    RETURN_IF_FAILED(pFwRule->put_Description(ruleDescription));
    RETURN_IF_FAILED(pFwRule->put_Enabled(VARIANT_TRUE));

    RETURN_IF_FAILED(pFwRule->put_Action(NET_FW_ACTION_ALLOW));

    std::wstring resolvedExecutableFullPath = m_msixRequest->GetPackageDirectoryPath() + L"\\" + m_msixRequest->GetPackageInfo()->GetDisplayName();
    BSTR applicationName = SysAllocString(resolvedExecutableFullPath.data());
    RETURN_IF_FAILED(pFwRule->put_ApplicationName(applicationName));

    //Map protocol
    RETURN_IF_FAILED(pFwRule->put_Protocol(ConvertToProtocol(firewallRule.protocol.c_str())));

    //Local ports
    if (!firewallRule.localPortMin.empty() && !firewallRule.localPortMax.empty() )
    {
        std::wstring localPortRange = firewallRule.localPortMin.c_str();
        localPortRange.append(L"-");
        localPortRange.append(firewallRule.localPortMax.c_str());
        BSTR localPorts = SysAllocString(localPortRange.data());
        RETURN_IF_FAILED(pFwRule->put_LocalPorts(localPorts));
    }

    //Remote ports
    if (!firewallRule.remotePortMin.empty() && !firewallRule.remotePortMax.empty())
    {
        std::wstring remotePortRange = firewallRule.remotePortMin.c_str();
        remotePortRange.append(L"-");
        remotePortRange.append(firewallRule.remotePortMax.c_str());
        BSTR remotePorts = SysAllocString(remotePortRange.data());
        RETURN_IF_FAILED(pFwRule->put_RemotePorts(remotePorts));
    }

    //Map direction
    if (_wcsicmp(firewallRule.direction.c_str(), directionIn.c_str()) == 0)
    {
        RETURN_IF_FAILED(pFwRule->put_Direction(NET_FW_RULE_DIR_IN));
    }
    else
    {
        RETURN_IF_FAILED(pFwRule->put_Direction(NET_FW_RULE_DIR_OUT));
    }

    //Map profile
    RETURN_IF_FAILED(pFwRule->put_Profiles(ConvertToProfileType(firewallRule.profile.c_str())));

    // Populate the Firewall Rule object
    RETURN_IF_FAILED(pFwRules->Add(pFwRule.Get()));

    return S_OK;
}

unsigned short FirewallRules::ConvertToProtocol(PCWSTR key)
{
    struct ProtocolMapping
    {
        PCWSTR key;
        unsigned short value;
    };

    ProtocolMapping protocolMap[] =
    {
        { L"ICMPv4",  1 },
        { L"ICMPv6",  58 },
        { L"TCP",     6 },
        { L"UDP",     17 },
        { L"GRE",     47 },
        { L"AH",      51 },
        { L"ESP",     50 },
        { L"EGP",     8 },
        { L"GGP",     3 },
        { L"HMP",     20 },
        { L"IGMP",    2 },
        { L"RVD",     66 },
        { L"OSPFIGP", 89 },
        { L"PUP",     12 },
        { L"RDP",     27 },
        { L"RSVP",    46 }
    };

    for (ProtocolMapping& mapping : protocolMap)
    {
        if (_wcsicmp(mapping.key, key) == 0)
        {
            return mapping.value;
        }
    }
    return NET_FW_IP_PROTOCOL_ANY;
}

NET_FW_PROFILE_TYPE2 FirewallRules::ConvertToProfileType(PCWSTR key)
{
    struct ProfileMapping
    {
        PCWSTR key;
        NET_FW_PROFILE_TYPE2 value;
    };

    ProfileMapping profileMap[] =
    {
        {L"domain", NET_FW_PROFILE2_DOMAIN},
        {L"private", NET_FW_PROFILE2_PRIVATE},
        {L"public", NET_FW_PROFILE2_PUBLIC},
        {L"domainAndPrivate", static_cast<NET_FW_PROFILE_TYPE2>(NET_FW_PROFILE2_DOMAIN | NET_FW_PROFILE2_PRIVATE)},
        {L"all", NET_FW_PROFILE2_ALL}
    };

    for (ProfileMapping& mapping : profileMap)
    {
        if (_wcsicmp(mapping.key, key) == 0)
        {
            return mapping.value;
        }
    }
    return NET_FW_PROFILE2_ALL;
}

HRESULT FirewallRules::WFCOMInitialize(INetFwPolicy2 ** ppNetFwPolicy2)
{
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(NetFwPolicy2), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwPolicy2), (void**)ppNetFwPolicy2));
    return S_OK;
}

HRESULT FirewallRules::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<FirewallRules > localInstance(new FirewallRules(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}


