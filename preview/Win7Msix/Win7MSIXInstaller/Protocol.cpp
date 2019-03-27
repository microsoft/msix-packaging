#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "Protocol.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR Protocol::HandlerName = L"Protocol";

HRESULT Protocol::ParseProtocolElement(IMsixElement* protocolElement)
{
    Text<wchar_t> protocolName;
    RETURN_IF_FAILED(protocolElement->GetAttributeValue(nameAttribute.c_str(), &protocolName));

    ProtocolData protocol;
    protocol.name = protocolName.Get();

    Text<wchar_t> parameters;
    RETURN_IF_FAILED(protocolElement->GetAttributeValue(parametersAttribute.c_str(), &parameters));
    protocol.parameters = parameters.Get();

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Parsing Protocol",
        TraceLoggingValue(protocolName.Get(), "Name"));

    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> logoEnum;
    RETURN_IF_FAILED(protocolElement->GetElements(logoQuery.c_str(), &logoEnum));
    RETURN_IF_FAILED(logoEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> logoElement;
        RETURN_IF_FAILED(logoEnum->GetCurrent(&logoElement));

        Text<wchar_t> logoPath;
        RETURN_IF_FAILED(logoElement->GetText(&logoPath));

        protocol.logo = m_msixRequest->GetPackageInfo()->GetPackageDirectoryPath() + std::wstring(L"\\") + logoPath.Get();
    }

    ComPtr<IMsixElementEnumerator> displayNameEnum;
    RETURN_IF_FAILED(protocolElement->GetElements(displayNameQuery.c_str(), &displayNameEnum));
    RETURN_IF_FAILED(displayNameEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> displayNameElement;
        RETURN_IF_FAILED(displayNameEnum->GetCurrent(&displayNameElement));

        Text<wchar_t> displayName;
        RETURN_IF_FAILED(displayNameElement->GetText(&displayName));

        protocol.displayName = urlProtocolPrefix + displayName.Get();
    }
    else
    {
        protocol.displayName = urlProtocolPrefix + protocol.name;
    }

    m_protocols.push_back(protocol);

    return S_OK;
}

HRESULT Protocol::ParseManifest()
{
    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(m_msixRequest->GetPackageInfo()->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    ComPtr<IMsixElementEnumerator> extensionEnum;
    RETURN_IF_FAILED(element->GetElements(extensionQuery.c_str(), &extensionEnum));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(extensionEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> extensionElement;
        RETURN_IF_FAILED(extensionEnum->GetCurrent(&extensionElement));
        Text<wchar_t> extensionCategory;
        RETURN_IF_FAILED(extensionElement->GetAttributeValue(categoryAttribute.c_str(), &extensionCategory));

        if (wcscmp(extensionCategory.Get(), protocolCategoryNameInManifest.c_str()) == 0)
        {
            BOOL hc_protocol;
            ComPtr<IMsixElementEnumerator> protocolEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(protocolQuery.c_str(), &protocolEnum));
            RETURN_IF_FAILED(protocolEnum->GetHasCurrent(&hc_protocol));

            if (hc_protocol)
            {
                ComPtr<IMsixElement> protocolElement;
                RETURN_IF_FAILED(protocolEnum->GetCurrent(&protocolElement));

                RETURN_IF_FAILED(ParseProtocolElement(protocolElement.Get()));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT Protocol::ExecuteForAddRequest()
{
    for (auto protocol = m_protocols.begin(); protocol != m_protocols.end(); ++protocol)
    {
        RETURN_IF_FAILED(ProcessProtocolForAdd(*protocol));
    }

    return S_OK;
}

HRESULT Protocol::ProcessProtocolForAdd(ProtocolData& protocol)
{
    RegistryKey protocolKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(protocol.name.c_str(), KEY_WRITE, &protocolKey));

    RETURN_IF_FAILED(protocolKey.SetValue(protocolValueName.c_str(), nullptr, 0, REG_SZ));
    RETURN_IF_FAILED(protocolKey.SetStringValue(L"", protocol.displayName));

    if (protocol.logo.c_str() != nullptr)
    {
        RegistryKey defaultIconKey;
        RETURN_IF_FAILED(protocolKey.CreateSubKey(defaultIconKeyName.c_str(), KEY_WRITE, &defaultIconKey));
        RETURN_IF_FAILED(defaultIconKey.SetStringValue(L"", protocol.logo.c_str()));
    }

    RegistryKey shellKey;
    RETURN_IF_FAILED(protocolKey.CreateSubKey(shellKeyName.c_str(), KEY_WRITE, &shellKey));
    RETURN_IF_FAILED(shellKey.SetStringValue(L"", openKeyName));

    RegistryKey openKey;
    RETURN_IF_FAILED(shellKey.CreateSubKey(openKeyName.c_str(), KEY_WRITE, &openKey));

    RegistryKey commandKey;
    RETURN_IF_FAILED(openKey.CreateSubKey(commandKeyName.c_str(), KEY_WRITE, &commandKey));

    std::wstring command = m_msixRequest->GetPackageInfo()->GetExecutableFilePath();
    if (protocol.parameters.c_str() != nullptr)
    {
        command += std::wstring(L" ") + protocol.parameters;
    }
    else
    {
        command += commandArgument;
    }
    RETURN_IF_FAILED(commandKey.SetStringValue(L"", command));

    return S_OK;
}

HRESULT Protocol::ExecuteForRemoveRequest()
{
    for (auto protocol = m_protocols.begin(); protocol != m_protocols.end(); ++protocol)
    {
        RETURN_IF_FAILED(ProcessProtocolForRemove(*protocol));
    }

    return S_OK;
}

HRESULT Protocol::ProcessProtocolForRemove(ProtocolData& protocol)
{
    HRESULT hrDeleteKey = m_classesKey.DeleteTree(protocol.name.c_str());
    if (FAILED(hrDeleteKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to delete protocol extension",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrDeleteKey, "HR"),
            TraceLoggingValue(protocol.name.c_str(), "Protocol"));
    }

    return S_OK;
}

HRESULT Protocol::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<Protocol> localInstance(new Protocol(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CLASSES_ROOT, nullptr, KEY_READ | KEY_WRITE | WRITE_DAC));
    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
