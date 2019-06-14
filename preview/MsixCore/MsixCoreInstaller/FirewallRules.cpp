#include <windows.h>
#include <stdio.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <experimental/filesystem>
#include <netfw.h>

#include "FirewallRules.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

using namespace MsixCoreLib;

const PCWSTR FirewallRules::HandlerName = L"FirewallRules";

HRESULT FirewallRules::ExecuteForAddRequest()
{
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
        ComPtr<IMsixElement> ruleElement;
        RETURN_IF_FAILED(firewallRuleEnum->GetCurrent(&ruleElement));

        Text<wchar_t> direction;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(ruleDirectionAttribute.c_str(), &direction));

        Text<wchar_t> protocol;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(ruleProtocolAttribute.c_str(), &protocol));

        Text<wchar_t> profile;
        RETURN_IF_FAILED(ruleElement->GetAttributeValue(ruleProfileAttribute.c_str(), &profile));

        INetFwPolicy2 *pNetFwPolicy2 = NULL;
        INetFwRules *pFwRules = NULL;
        INetFwRule *pFwRule = NULL;

        // Retrieve INetFwPolicy2
        RETURN_IF_FAILED(WFCOMInitialize(&pNetFwPolicy2));

        // Retrieve INetFwRules
        RETURN_IF_FAILED(pNetFwPolicy2->get_Rules(&pFwRules));

        // Create a new Firewall Rule object.
        RETURN_IF_FAILED(CoCreateInstance(__uuidof(NetFwRule), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), (void**)&pFwRule));

        std::wstring packageDisplayName = m_msixRequest->GetPackageInfo()->GetDisplayName();
        BSTR ruleName = SysAllocString(packageDisplayName.data());
        BSTR ruleDescription = SysAllocString(packageDisplayName.data());
        pFwRule->put_Name(ruleName);
        pFwRule->put_Description(ruleDescription);
        pFwRule->put_Enabled(VARIANT_TRUE);

        pFwRule->put_Action(NET_FW_ACTION_ALLOW);

        std::wstring resolvedExecutableFullPath = m_msixRequest->GetPackageDirectoryPath() + L"\\" + m_msixRequest->GetPackageInfo()->GetDisplayName();
        BSTR applicationName = SysAllocString(resolvedExecutableFullPath.data());
        pFwRule->put_ApplicationName(applicationName);

        //Map protocol
        pFwRule->put_Protocol(ConvertToProtocol(protocol.Get()));

        std::wstring portNums = L"80";
        BSTR localPorts = SysAllocString(portNums.data());
        HRESULT portsResult = pFwRule->put_LocalPorts(localPorts);
        pFwRule->put_RemotePorts(localPorts);

        //Map direction
        if (_wcsicmp(direction.Get(), directionIn.c_str()) == 0)
        {
            pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
        }
        else
        {
            pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
        }

        //Map profile
        pFwRule->put_Profiles(ConvertToProfileType(profile.Get()));

        // Populate the Firewall Rule object
        HRESULT hr = pFwRules->Add(pFwRule);
        if (SUCCEEDED(hr))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Success",
                TraceLoggingValue(hr, "HR"));
        }
        else
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Failure",
                TraceLoggingValue(hr, "HR"));
        }

        RETURN_IF_FAILED(firewallRuleEnum->MoveNext(&hasCurrent));
    }

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
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)ppNetFwPolicy2);

    if (FAILED(hr))
    {
        printf("CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

Cleanup:
    return hr;
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

HRESULT FirewallRules::ExecuteForRemoveRequest()
{
    return S_OK;
}


