#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ComServer.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR ComServer::HandlerName = L"ComServer";

static const std::wstring msix7ProgIDPrefix = L"Msix7";
static const std::wstring openWithProgIdsKeyName = L"OpenWithProgids";
static const std::wstring shellKeyName = L"Shell";
static const std::wstring openKeyName = L"open";
static const std::wstring commandKeyName = L"command";
static const std::wstring defaultIconKeyName = L"DefaultIcon";
static const std::wstring commandArgument = L" \"%1\"";

static const std::wstring clsidKeyName = L"CLSID";

static const std::wstring ComServerCategoryNameInManifest = L"windows.comServer";

static const std::wstring categoryAttribute = L"Category";
static const std::wstring idAttribute = L"Id";
static const std::wstring executableAttribute = L"Executable";
static const std::wstring argumentsAttribute = L"Arguments";
static const std::wstring displayNameAttribute = L"DisplayName";
static const std::wstring launchAndActivationPermissionAttribute = L"LaunchAndActivationPermission";
static const std::wstring progIdAttribute = L"ProgId";
static const std::wstring versionIndependentProgIdAttribute = L"VersionIndependentProgId";
static const std::wstring clsidAttribute = L"Clsid";
static const std::wstring currentVersionAttribute = L"CurrentVersion";

static const std::wstring inprocHandlerKeyName = L"InprocHandler32";
static const std::wstring defaultInprocHandler = L"ole32.dll";
static const std::wstring localServerKeyName = L"LocalServer32";
static const std::wstring progIdKeyName = L"ProgID";
static const std::wstring versionIndependentProgIdKeyName = L"VersionIndependentProgID";
static const std::wstring curVerKeyName = L"CurVer";

static const std::wstring extensionQuery = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']";
static const std::wstring exeServerQuery = L"*[local-name()='ComServer']/*[local-name()='ExeServer']";
static const std::wstring exeServerClassQuery = L"*[local-name()='Class']";
static const std::wstring progIdQuery = L"*[local-name()='ComServer']/*[local-name()='ProgId']";


HRESULT ComServer::ExecuteForAddRequest()
{
    for (auto exeServer = m_exeServers.begin(); exeServer != m_exeServers.end(); ++exeServer)
    {
        RETURN_IF_FAILED(ProcessExeServerForAdd(*exeServer));
    }

    for (auto progId = m_progIds.begin(); progId != m_progIds.end(); ++progId)
    {
        RETURN_IF_FAILED(ProcessProgIdForAdd(*progId));
    }

    return S_OK;
}

HRESULT ComServer::ProcessExeServerForAdd(ExeServer& exeServer)
{
    RegistryKey clsidKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(clsidKeyName.c_str(), KEY_WRITE, &clsidKey));

    for (auto exeServerClass = exeServer.classes.begin(); exeServerClass != exeServer.classes.end(); ++exeServerClass)
    {
        RegistryKey classIdKey;
        RETURN_IF_FAILED(clsidKey.CreateSubKey(exeServerClass->id.c_str(), KEY_WRITE, &classIdKey));
        RETURN_IF_FAILED(classIdKey.SetStringValue(L"", exeServerClass->displayName));

        if (false )//TODO this is enableOleDefaultHandler=true
        {
            RegistryKey inprocHandlerKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(inprocHandlerKeyName.c_str(), KEY_WRITE, &inprocHandlerKey));
            RETURN_IF_FAILED(inprocHandlerKey.SetStringValue(L"", defaultInprocHandler));
        }

        std::wstring executableFullPath = m_msixRequest->GetFilePathMappings()->GetExecutablePath(exeServer.executable, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
        RegistryKey localServerKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(localServerKeyName.c_str(), KEY_WRITE, &localServerKey));
        RETURN_IF_FAILED(localServerKey.SetStringValue(L"", executableFullPath));

        RegistryKey progIdKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(progIdKeyName.c_str(), KEY_WRITE, &progIdKey));
        RETURN_IF_FAILED(progIdKey.SetStringValue(L"", exeServerClass->progId));

        RegistryKey versionIndependentProgIdKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(versionIndependentProgIdKeyName.c_str(), KEY_WRITE, &versionIndependentProgIdKey));
        RETURN_IF_FAILED(versionIndependentProgIdKey.SetStringValue(L"", exeServerClass->versionIndependentProgId));
    }

    return S_OK;
}

HRESULT ComServer::ProcessProgIdForAdd(ProgId& progId)
{
    RegistryKey progIdKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(progId.id.c_str(), KEY_WRITE, &progIdKey));

    if (!progId.clsid.empty())
    {
        RegistryKey clsidKey;
        RETURN_IF_FAILED(progIdKey.CreateSubKey(clsidKeyName.c_str(), KEY_WRITE, &clsidKey));
        RETURN_IF_FAILED(clsidKey.SetStringValue(L"", progId.clsid));
    }
    else
    {
        RegistryKey curVerKey;
        RETURN_IF_FAILED(progIdKey.CreateSubKey(curVerKeyName.c_str(), KEY_WRITE, &curVerKey));
        RETURN_IF_FAILED(curVerKey.SetStringValue(L"", progId.currentVersion));
    }
    return S_OK;
}


HRESULT ComServer::ParseManifest()
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

        if (wcscmp(extensionCategory.Get(), ComServerCategoryNameInManifest.c_str()) == 0)
        {
            BOOL hc_exeServer = FALSE;
            ComPtr<IMsixElementEnumerator> exeServerEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(exeServerQuery.c_str(), &exeServerEnum));
            RETURN_IF_FAILED(exeServerEnum->GetHasCurrent(&hc_exeServer));

            while (hc_exeServer)
            {
                ComPtr<IMsixElement> exeServerElement;
                RETURN_IF_FAILED(exeServerEnum->GetCurrent(&exeServerElement));

                RETURN_IF_FAILED(ParseExeServerElement(exeServerElement.Get()));

                RETURN_IF_FAILED(exeServerEnum->MoveNext(&hc_exeServer));
            }

            BOOL hc_progId = FALSE;
            ComPtr<IMsixElementEnumerator> progIdEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(progIdQuery.c_str(), &progIdEnum));
            RETURN_IF_FAILED(progIdEnum->GetHasCurrent(&hc_progId));

            while (hc_progId)
            {
                ComPtr<IMsixElement> progIdElement;
                RETURN_IF_FAILED(progIdEnum->GetCurrent(&progIdElement));

                RETURN_IF_FAILED(ParseProgIdElement(progIdElement.Get()));

                RETURN_IF_FAILED(progIdEnum->MoveNext(&hc_progId));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseExeServerElement(IMsixElement* exeServerElement)
{
    ExeServer exeServer;

    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, executableAttribute, exeServer.executable));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, argumentsAttribute, exeServer.arguments));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, displayNameAttribute, exeServer.displayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, launchAndActivationPermissionAttribute, exeServer.launchAndActivationPermission));
    
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> classesEnum;
    RETURN_IF_FAILED(exeServerElement->GetElements(exeServerClassQuery.c_str(), &classesEnum));
    RETURN_IF_FAILED(classesEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> classElement;
        RETURN_IF_FAILED(classesEnum->GetCurrent(&classElement));

        RETURN_IF_FAILED(ParseExeServerClassElement(exeServer, classElement.Get()));
        RETURN_IF_FAILED(classesEnum->MoveNext(&hasCurrent));
    }

    m_exeServers.push_back(exeServer);

    return S_OK;
}

HRESULT ComServer::ParseProgIdElement(IMsixElement* progIdElement)
{
    ProgId progId;

    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, idAttribute, progId.id));
    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, clsidAttribute, id));
    if (!id.empty())
    {
        progId.clsid = GuidFromManifestId(id);
    }
    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, currentVersionAttribute, progId.currentVersion));

    m_progIds.push_back(progId);

    return S_OK;
}

HRESULT ComServer::ParseExeServerClassElement(ExeServer & exeServer, IMsixElement * classElement)
{
    ExeServerClass exeServerClass;

    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, idAttribute, id));
    exeServerClass.id = GuidFromManifestId(id);
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, displayNameAttribute, exeServerClass.displayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, progIdAttribute, exeServerClass.progId));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, versionIndependentProgIdAttribute, exeServerClass.versionIndependentProgId));

    exeServer.classes.push_back(exeServerClass);
    return S_OK;
}

HRESULT ComServer::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ComServer> localInstance(new ComServer(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CLASSES_ROOT, nullptr, KEY_READ | KEY_WRITE | WRITE_DAC));

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
