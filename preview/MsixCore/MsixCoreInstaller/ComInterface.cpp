#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ComInterface.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
using namespace MsixCoreLib;

const PCWSTR ComInterface::HandlerName = L"ComInterface";

HRESULT ComInterface::ExecuteForAddRequest()
{
    for (auto comInterface = m_interfaces.begin(); comInterface != m_interfaces.end(); ++comInterface)
    {
        RETURN_IF_FAILED(ProcessInterfaceForAddRequest(*comInterface));
    }

    for (auto typeLib = m_typeLibs.begin(); typeLib != m_typeLibs.end(); ++typeLib)
    {
        RETURN_IF_FAILED(ProcessTypeLibForAddRequest(*typeLib));
    }

    return S_OK;
}

HRESULT ComInterface::ExecuteForRemoveRequest()
{
    for (auto comInterface = m_interfaces.begin(); comInterface != m_interfaces.end(); ++comInterface)
    {
        RETURN_IF_FAILED(ProcessInterfaceForRemoveRequest(*comInterface));
    }

    for (auto typeLib = m_typeLibs.begin(); typeLib != m_typeLibs.end(); ++typeLib)
    {
        RETURN_IF_FAILED(ProcessTypeLibForRemoveRequest(*typeLib));
    }

    return S_OK;
}

HRESULT ComInterface::ProcessInterfaceForAddRequest(Interface& comInterface)
{
    RegistryKey interfaceKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(interfaceKeyName.c_str(), KEY_WRITE, &interfaceKey));

    RegistryKey interfaceIdKey;
    RETURN_IF_FAILED(interfaceKey.CreateSubKey(comInterface.id.c_str(), KEY_WRITE, &interfaceIdKey));

    RegistryKey proxyStubClsidKey;
    RETURN_IF_FAILED(interfaceIdKey.CreateSubKey(proxyStubClsidKeyName.c_str(), KEY_WRITE, &proxyStubClsidKey));
    RETURN_IF_FAILED(proxyStubClsidKey.SetStringValue(L"", comInterface.proxyStubClsid));

    RegistryKey typeLibKey;
    RETURN_IF_FAILED(interfaceIdKey.CreateSubKey(typeLibKeyName.c_str(), KEY_WRITE, &typeLibKey));
    RETURN_IF_FAILED(typeLibKey.SetStringValue(L"", comInterface.typeLibId));
    RETURN_IF_FAILED(typeLibKey.SetStringValue(versionValueName.c_str(), comInterface.typeLibVersion));
    return S_OK;
}

HRESULT ComInterface::ProcessTypeLibForAddRequest(TypeLib& typeLib)
{
    RegistryKey typeLibKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(typeLibKeyName.c_str(), KEY_WRITE, &typeLibKey));

    RegistryKey typeLibIdKey;
    RETURN_IF_FAILED(typeLibKey.CreateSubKey(typeLib.id.c_str(), KEY_WRITE, &typeLibIdKey));

    for (auto version = typeLib.version.begin(); version != typeLib.version.end(); ++version)
    {
        RegistryKey versionNumberKey;
        RETURN_IF_FAILED(typeLibIdKey.CreateSubKey(version->versionNumber.c_str(), KEY_WRITE, &versionNumberKey));
        if (!version->displayName.empty())
        {
            RETURN_IF_FAILED(versionNumberKey.SetStringValue(L"", version->displayName));
        }

        RegistryKey localeIdKey;
        RETURN_IF_FAILED(versionNumberKey.CreateSubKey(version->localeId.c_str(), KEY_WRITE, &localeIdKey));

        if (!version->win32Path.empty())
        {
            RegistryKey win32Key;
            RETURN_IF_FAILED(localeIdKey.CreateSubKey(win32KeyName.c_str(), KEY_WRITE, &win32Key));

            std::wstring win32FullPath = FilePathMappings::GetInstance().GetExecutablePath(version->win32Path, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
            RETURN_IF_FAILED(win32Key.SetStringValue(L"", win32FullPath));
        }

        if (!version->win64Path.empty())
        {
            RegistryKey win64Key;
            RETURN_IF_FAILED(localeIdKey.CreateSubKey(win64KeyName.c_str(), KEY_WRITE, &win64Key));

            std::wstring win64FullPath = FilePathMappings::GetInstance().GetExecutablePath(version->win64Path, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
            RETURN_IF_FAILED(win64Key.SetStringValue(L"", win64FullPath));
        }

        if (!version->libraryFlag.empty())
        {
            RegistryKey flagsKey;
            RETURN_IF_FAILED(versionNumberKey.CreateSubKey(flagsKeyName.c_str(), KEY_WRITE, &flagsKey));
            RETURN_IF_FAILED(flagsKey.SetStringValue(L"", version->libraryFlag));
        }

        if (!version->helpDirectory.empty())
        {
            RegistryKey helpDirKey;
            RETURN_IF_FAILED(versionNumberKey.CreateSubKey(helpDirKeyName.c_str(), KEY_WRITE, &helpDirKey));

            std::wstring helpDirFullPath = FilePathMappings::GetInstance().GetExecutablePath(version->helpDirectory, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
            RETURN_IF_FAILED(helpDirKey.SetStringValue(L"", helpDirFullPath));
        }
    }

    return S_OK;
}

HRESULT ComInterface::ProcessInterfaceForRemoveRequest(Interface& comInterface)
{
    RegistryKey interfaceKey;
    RETURN_IF_FAILED(m_classesKey.OpenSubKey(interfaceKeyName.c_str(), KEY_WRITE, &interfaceKey));

    const HRESULT hrDeleteKey = interfaceKey.DeleteTree(comInterface.id.c_str());
    if (FAILED(hrDeleteKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to delete com interface",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrDeleteKey, "HR"),
            TraceLoggingValue(comInterface.id.c_str(), "interface"));
    }

    return S_OK;
}

HRESULT ComInterface::ProcessTypeLibForRemoveRequest(TypeLib& typeLib)
{
    RegistryKey typeLibKey;
    RETURN_IF_FAILED(m_classesKey.OpenSubKey(typeLibKeyName.c_str(), KEY_WRITE, &typeLibKey));

    const HRESULT hrDeleteKey = typeLibKey.DeleteTree(typeLib.id.c_str());
    if (FAILED(hrDeleteKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to delete com interface typeLib",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrDeleteKey, "HR"),
            TraceLoggingValue(typeLib.id.c_str(), "interface"));
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

            ComPtr<IMsixElementEnumerator> typeLibEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(typeLibQuery.c_str(), &typeLibEnum));
            RETURN_IF_FAILED(typeLibEnum->GetHasCurrent(&hc_extension));

            while (hc_extension)
            {
                ComPtr<IMsixElement> typeLibElement;
                RETURN_IF_FAILED(typeLibEnum->GetCurrent(&typeLibElement));

                RETURN_IF_FAILED(ParseTypeLibElement(typeLibElement.Get()));

                RETURN_IF_FAILED(typeLibEnum->MoveNext(&hc_extension));
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

    std::wstring clsid;
    RETURN_IF_FAILED(GetAttributeValueFromElement(interfaceElement, proxyStubClsidAttribute, clsid));

    if (!clsid.empty())
    {
        comInterface.proxyStubClsid = GuidFromManifestId(clsid);
    }
    else
    {
        comInterface.proxyStubClsid = proxyStubClsidForUniversalMarshaler;

        BOOL hasCurrent = FALSE;
        ComPtr<IMsixElementEnumerator> typelibEnum;
        RETURN_IF_FAILED(interfaceElement->GetElements(typeLibForInterfaceQuery.c_str(), &typelibEnum));
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
    }
    m_interfaces.push_back(comInterface);

    return S_OK;
}

HRESULT ComInterface::ParseTypeLibElement(IMsixElement * typeLibElement)
{
    TypeLib typeLib;

    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(typeLibElement, idAttribute, id));
    typeLib.id = GuidFromManifestId(id);

    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> versionEnum;
    RETURN_IF_FAILED(typeLibElement->GetElements(versionQuery.c_str(), &versionEnum));
    RETURN_IF_FAILED(versionEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> versionElement;
        RETURN_IF_FAILED(versionEnum->GetCurrent(&versionElement));

        RETURN_IF_FAILED(ParseVersionElement(typeLib, versionElement.Get()));

        RETURN_IF_FAILED(versionEnum->MoveNext(&hasCurrent));
    }

    m_typeLibs.push_back(typeLib);
    return S_OK;
}


HRESULT ComInterface::ParseVersionElement(TypeLib & typeLib, IMsixElement * versionElement)
{
    Version version;

    RETURN_IF_FAILED(GetAttributeValueFromElement(versionElement, displayNameAttribute, version.displayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(versionElement, versionNumberAttribute, version.versionNumber));
    RETURN_IF_FAILED(GetAttributeValueFromElement(versionElement, localeIdAttribute, version.localeId));
    RETURN_IF_FAILED(GetAttributeValueFromElement(versionElement, libraryFlagAttribute, version.libraryFlag));
    RETURN_IF_FAILED(GetAttributeValueFromElement(versionElement, helpDirectoryAttribute, version.helpDirectory));

    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> pathEnum;
    RETURN_IF_FAILED(versionElement->GetElements(win32PathQuery.c_str(), &pathEnum));
    RETURN_IF_FAILED(pathEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> pathElement;
        RETURN_IF_FAILED(pathEnum->GetCurrent(&pathElement));
        RETURN_IF_FAILED(GetAttributeValueFromElement(pathElement.Get(), pathAttribute, version.win32Path));
    }

    RETURN_IF_FAILED(versionElement->GetElements(win64PathQuery.c_str(), &pathEnum));
    RETURN_IF_FAILED(pathEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> pathElement;
        RETURN_IF_FAILED(pathEnum->GetCurrent(&pathElement));
        RETURN_IF_FAILED(GetAttributeValueFromElement(pathElement.Get(), pathAttribute, version.win64Path));
    }

    typeLib.version.push_back(version);
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