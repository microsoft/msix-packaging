#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "FileTypeAssociation.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include "RegistryKey.hpp"
#include "MsixTraceLoggingProvider.hpp"
using namespace MsixCoreLib;

const PCWSTR FileTypeAssociation::HandlerName = L"FileTypeAssociation";

std::wstring FileTypeAssociation::CreateProgID(PCWSTR name)
{
    std::wstring packageFullName = m_msixRequest->GetPackageInfo()->GetPackageFullName();
    std::wstring progID = msixCoreProgIDPrefix + packageFullName.substr(0, packageFullName.find(L"_")) + name;
    return progID;
}

HRESULT FileTypeAssociation::ParseFtaElement(IMsixElement* ftaElement)
{
    Text<wchar_t> ftaName;
    RETURN_IF_FAILED(ftaElement->GetAttributeValue(nameAttribute.c_str(), &ftaName));

    std::wstring name = L"." + std::wstring(ftaName.Get());
    std::wstring progID = CreateProgID(name.c_str());

    Fta fta;
    fta.name = name;
    fta.progID = progID;

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Parsing Fta",
        TraceLoggingValue(ftaName.Get(), "Name"));

    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> ftaEnum;
    RETURN_IF_FAILED(ftaElement->GetElements(fileTypeQuery.c_str(), &ftaEnum));
    RETURN_IF_FAILED(ftaEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> fileTypeElement;
        RETURN_IF_FAILED(ftaEnum->GetCurrent(&fileTypeElement));

        Text<wchar_t> extension;
        RETURN_IF_FAILED(fileTypeElement->GetText(&extension));

        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Extension",
            TraceLoggingValue(extension.Get(), "Extension"));
        fta.extensions.push_back(extension.Get());

        RETURN_IF_FAILED(ftaEnum->MoveNext(&hasCurrent));
    }

    ComPtr<IMsixElementEnumerator> logoEnum;
    RETURN_IF_FAILED(ftaElement->GetElements(logoQuery.c_str(), &logoEnum));
    RETURN_IF_FAILED(logoEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> logoElement;
        RETURN_IF_FAILED(logoEnum->GetCurrent(&logoElement));

        Text<wchar_t> logoPath;
        RETURN_IF_FAILED(logoElement->GetText(&logoPath));

        fta.logo = m_msixRequest->GetPackageDirectoryPath() + std::wstring(L"\\") + logoPath.Get();
    }

    ComPtr<IMsixElementEnumerator> verbsEnum;
    RETURN_IF_FAILED(ftaElement->GetElements(verbQuery.c_str(), &verbsEnum));
    RETURN_IF_FAILED(verbsEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> verbElement;
        RETURN_IF_FAILED(verbsEnum->GetCurrent(&verbElement));

        Text<wchar_t> verbName;
        RETURN_IF_FAILED(verbElement->GetText(&verbName));

        Text<wchar_t> parameters;
        RETURN_IF_FAILED(verbElement->GetAttributeValue(parametersAttribute.c_str(), &parameters));

        Verb verb;
        verb.verb = verbName.Get();
        verb.parameter = parameters.Get();
        fta.verbs.push_back(verb);

        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Verb",
            TraceLoggingValue(verbName.Get(), "Verb"),
            TraceLoggingValue(parameters.Get(), "Parameter"));

        RETURN_IF_FAILED(verbsEnum->MoveNext(&hasCurrent));
    }

    m_Ftas.push_back(fta);

    return S_OK;
}

HRESULT FileTypeAssociation::ParseManifest()
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

        if (wcscmp(extensionCategory.Get(), ftaCategoryNameInManifest.c_str()) == 0)
        {
            BOOL hc_fta;
            ComPtr<IMsixElementEnumerator> ftaEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(ftaQuery.c_str(), &ftaEnum));
            RETURN_IF_FAILED(ftaEnum->GetHasCurrent(&hc_fta));

            if (hc_fta)
            {
                ComPtr<IMsixElement> ftaElement;
                RETURN_IF_FAILED(ftaEnum->GetCurrent(&ftaElement));

                RETURN_IF_FAILED(ParseFtaElement(ftaElement.Get()));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::ExecuteForAddRequest()
{
    for (auto fta = m_Ftas.begin(); fta != m_Ftas.end(); ++fta)
    {
        RETURN_IF_FAILED(ProcessFtaForAdd(*fta));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::ProcessFtaForAdd(Fta& fta)
{
    bool needToProcessAnyExtensions = false;
    for (auto extensionName = fta.extensions.begin(); extensionName != fta.extensions.end(); ++extensionName)
    {
        if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
        {
            return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }

        //bool registryHasExtension = false;
        //RETURN_IF_FAILED(m_registryDevirtualizer->HasFTA(*extensionName, registryHasExtension));

        bool registryHasExtension = false;
        RETURN_IF_FAILED(m_registryDevirtualizer->HasRegistryKey(HKEY_CURRENT_USER, classesKeyPath.c_str(), *extensionName, registryHasExtension));

        if (registryHasExtension)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Registry devirtualization already wrote an entry for this extension -- not processing extension",
                TraceLoggingValue(extensionName->c_str(), "Extension"));
        }
        else
        {
            needToProcessAnyExtensions = true;
            RegistryKey ftaKey;
            RETURN_IF_FAILED(m_classesKey.CreateSubKey(extensionName->c_str(), KEY_WRITE, &ftaKey));
            RETURN_IF_FAILED(ftaKey.SetStringValue(L"", fta.progID));

            RegistryKey openWithProgIdsKey;
            RETURN_IF_FAILED(ftaKey.CreateSubKey(openWithProgIdsKeyName.c_str(), KEY_WRITE, &openWithProgIdsKey));
            RETURN_IF_FAILED(openWithProgIdsKey.SetValue(fta.progID.c_str(), nullptr, 0, REG_NONE));
        }
    }

    if (!needToProcessAnyExtensions)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Registry devirtualization already wrote entries for all extensions associated with this FTA, nothing more to process for this FTA",
            TraceLoggingValue(fta.name.c_str(), "Name"));
        return S_OK;
    }

    RegistryKey progIdKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(fta.progID.c_str(), KEY_WRITE, &progIdKey));

    if (fta.logo.c_str() != nullptr)
    {
        RegistryKey defaultIconKey;
        RETURN_IF_FAILED(progIdKey.CreateSubKey(defaultIconKeyName.c_str(), KEY_WRITE, &defaultIconKey));
        RETURN_IF_FAILED(defaultIconKey.SetStringValue(L"", fta.logo.c_str()));
    }

    RegistryKey shellKey;
    RETURN_IF_FAILED(progIdKey.CreateSubKey(shellKeyName.c_str(), KEY_WRITE, &shellKey));
    RETURN_IF_FAILED(shellKey.SetStringValue(L"", openKeyName));

    RegistryKey openKey;
    RETURN_IF_FAILED(shellKey.CreateSubKey(openKeyName.c_str(), KEY_WRITE, &openKey));

    RegistryKey commandKey;
    RETURN_IF_FAILED(openKey.CreateSubKey(commandKeyName.c_str(), KEY_WRITE, &commandKey));

    std::wstring executablePath = m_msixRequest->GetPackageDirectoryPath() + L"\\" + m_msixRequest->GetPackageInfo()->GetRelativeExecutableFilePath();
    std::wstring command = executablePath + commandArgument;
    RETURN_IF_FAILED(commandKey.SetStringValue(L"", command));

    for (auto verb = fta.verbs.begin(); verb != fta.verbs.end(); ++verb)
    {
        RegistryKey verbKey;
        RETURN_IF_FAILED(shellKey.CreateSubKey(verb->verb.c_str(), KEY_WRITE, &verbKey));

        RegistryKey verbCommandKey;
        RETURN_IF_FAILED(verbKey.CreateSubKey(commandKeyName.c_str(), KEY_WRITE, &verbCommandKey));

        std::wstring verbCommand = executablePath;
        if (verb->parameter.c_str() != nullptr)
        {
            verbCommand += std::wstring(L" ") + verb->parameter.c_str();
        }
        RETURN_IF_FAILED(verbCommandKey.SetStringValue(L"", verbCommand));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::ExecuteForRemoveRequest()
{
    for (auto fta = m_Ftas.begin(); fta != m_Ftas.end(); ++fta)
    {
        RETURN_IF_FAILED(ProcessFtaForRemove(*fta));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::ProcessFtaForRemove(Fta& fta)
{
    bool needToProcessAnyExtensions = false;
    for (auto extensionName = fta.extensions.begin(); extensionName != fta.extensions.end(); ++extensionName)
    {
        bool registryHasExtension = false;
        RETURN_IF_FAILED(m_registryDevirtualizer->HasFTA(*extensionName, registryHasExtension));

        if (registryHasExtension)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Registry devirtualization already wrote an entry for this extension -- not processing extension",
                TraceLoggingValue(extensionName->c_str(), "Extension"));
        }
        else
        {
            needToProcessAnyExtensions = true;

            HRESULT hrDeleteKey = m_classesKey.DeleteTree(extensionName->c_str());
            if (FAILED(hrDeleteKey))
            {
                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "Unable to delete extension",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(hrDeleteKey, "HR"),
                    TraceLoggingValue(extensionName->c_str(), "Extension"));
            }
        }
    }

    if (!needToProcessAnyExtensions)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Registry devirtualization already wrote entries for all extensions associated with this FTA, nothing more to process for this FTA",
            TraceLoggingValue(fta.name.c_str(), "Name"));
        return S_OK;
    }

    HRESULT hrDeleteKey = m_classesKey.DeleteTree(fta.progID.c_str());
    if (FAILED(hrDeleteKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to delete extension",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrDeleteKey, "HR"),
            TraceLoggingValue(fta.progID.c_str(), "ProgID"));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<FileTypeAssociation> localInstance(new FileTypeAssociation(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    //RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CLASSES_ROOT, nullptr, KEY_READ | KEY_WRITE | WRITE_DAC));
    RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CURRENT_USER, classesKeyPath.c_str(), KEY_READ | KEY_WRITE | WRITE_DAC));

    std::wstring registryFilePath = msixRequest->GetPackageDirectoryPath() + registryDatFile;
    RETURN_IF_FAILED(RegistryDevirtualizer::Create(registryFilePath, msixRequest, &localInstance->m_registryDevirtualizer));

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
