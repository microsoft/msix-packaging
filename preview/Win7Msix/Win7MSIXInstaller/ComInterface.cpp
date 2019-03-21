#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ComInterface.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR ComInterface::HandlerName = L"ComInterface";

static const std::wstring msix7ProgIDPrefix = L"Msix7";
static const std::wstring openWithProgIdsKeyName = L"OpenWithProgids";
static const std::wstring shellKeyName = L"Shell";
static const std::wstring openKeyName = L"open";
static const std::wstring commandKeyName = L"command";
static const std::wstring defaultIconKeyName = L"DefaultIcon";
static const std::wstring commandArgument = L" \"%1\"";

static const std::wstring comInterfaceCategoryNameInManifest = L"windows.comInterface";

static const std::wstring categoryAttribute = L"Category";
static const std::wstring idAttribute = L"Id";
static const std::wstring versionNumberAttribute = L"VersionNumber";
static const std::wstring proxyStubClsidAttribute = L"ProxyStubClsid";
static const std::wstring proxyStubClsidForUniversalMarshaler = L"{00020424-0000-0000-C000-000000000046}";

static const std::wstring extensionQuery = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']";
static const std::wstring interfaceQuery = L"*[local-name()='ComInterface']/*[local-name()='Interface']";
static const std::wstring typelibForInterfaceQuery = L"*[local-name()='Typelib']";

static const std::wstring interfaceKeyName = L"Interface";
static const std::wstring proxyStubClsidKeyName = L"ProxyStubClsid32";
static const std::wstring typeLibKeyName = L"TypeLib";
static const std::wstring versionValueName = L"Version";

HRESULT ComInterface::ExecuteForAddRequest()
{
    RegistryKey interfaceKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(interfaceKeyName.c_str(), KEY_WRITE, &interfaceKey));

    for (auto comInterface = m_interfaces.begin(); comInterface != m_interfaces.end(); ++comInterface)
    {
        RegistryKey interfaceIdKey;
        RETURN_IF_FAILED(interfaceKey.CreateSubKey(comInterface->id.c_str(), KEY_WRITE, &interfaceIdKey));

        RegistryKey proxyStubClsidKey;
        RETURN_IF_FAILED(interfaceIdKey.CreateSubKey(proxyStubClsidKeyName.c_str(), KEY_WRITE, &proxyStubClsidKey));
        RETURN_IF_FAILED(proxyStubClsidKey.SetStringValue(L"", comInterface->proxyStubClsid));

        RegistryKey typeLibKey;
        RETURN_IF_FAILED(interfaceIdKey.CreateSubKey(typeLibKeyName.c_str(), KEY_WRITE, &typeLibKey));
        RETURN_IF_FAILED(typeLibKey.SetStringValue(L"", comInterface->typeLibId));
        RETURN_IF_FAILED(typeLibKey.SetStringValue(versionValueName.c_str(), comInterface->typeLibVersion));
    }

    return S_OK;
}

HRESULT ComInterface::ParseManifest()
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

        if (wcscmp(extensionCategory.Get(), comInterfaceCategoryNameInManifest.c_str()) == 0)
        {
            BOOL hc_extension = FALSE;
            ComPtr<IMsixElementEnumerator> comInterfaceEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(interfaceQuery.c_str(), &comInterfaceEnum));
            RETURN_IF_FAILED(comInterfaceEnum->GetHasCurrent(&hc_extension));

            while (hc_extension)
            {
                ComPtr<IMsixElement> comInterfaceElement;
                RETURN_IF_FAILED(comInterfaceEnum->GetCurrent(&comInterfaceElement));

                RETURN_IF_FAILED(ParseComInterfaceElement(comInterfaceElement.Get()));

                RETURN_IF_FAILED(comInterfaceEnum->MoveNext(&hc_extension));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComInterface::ParseComInterfaceElement(IMsixElement* interfaceElement)
{
    Interface comInterface;
    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(interfaceElement, idAttribute, id));
    comInterface.id = GuidFromManifestId(id);
    RETURN_IF_FAILED(GetAttributeValueFromElement(interfaceElement, proxyStubClsidAttribute, id));

    if (!id.empty())
    {
        comInterface.proxyStubClsid = GuidFromManifestId(id);
        return S_OK;
    }

    comInterface.proxyStubClsid = proxyStubClsidForUniversalMarshaler;

    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> typelibEnum;
    RETURN_IF_FAILED(interfaceElement->GetElements(typelibForInterfaceQuery.c_str(), &typelibEnum));
    RETURN_IF_FAILED(typelibEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> typeLibElement;
        RETURN_IF_FAILED(typelibEnum->GetCurrent(&typeLibElement));

        RETURN_IF_FAILED(GetAttributeValueFromElement(typeLibElement.Get(), idAttribute, id));
        comInterface.typeLibId = GuidFromManifestId(id);
        RETURN_IF_FAILED(GetAttributeValueFromElement(typeLibElement.Get(), versionNumberAttribute, comInterface.typeLibVersion));
        
        RETURN_IF_FAILED(typelibEnum->MoveNext(&hasCurrent));
    }

    m_interfaces.push_back(comInterface);

    return S_OK;
}

HRESULT ComInterface::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ComInterface> localInstance(new ComInterface(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CLASSES_ROOT, nullptr, KEY_READ | KEY_WRITE | WRITE_DAC));

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
