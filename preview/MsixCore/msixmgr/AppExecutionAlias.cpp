#include <windows.h>
#include "GeneralUtil.hpp"
#include "RegistryKey.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"
#include "AppExecutionAlias.hpp"

using namespace MsixCoreLib;

const PCWSTR AppExecutionAlias::HandlerName = L"AppExecutionAlias";

HRESULT AppExecutionAlias::ExecuteForAddRequest()
{
    for (auto executionAlias = m_appExecutionAliases.begin(); executionAlias != m_appExecutionAliases.end(); ++executionAlias)
    {
        RETURN_IF_FAILED(ProcessAliasForAdd(*executionAlias));
    }
    return S_OK;
}

HRESULT AppExecutionAlias::ExecuteForRemoveRequest()
{
    return S_OK;
}

HRESULT MsixCoreLib::AppExecutionAlias::ParseManifest()
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

        if (wcscmp(extensionCategory.Get(), appExecutionAliasCategory.c_str()) == 0)
        {
            BOOL hc_executionAlias = FALSE;
            ComPtr<IMsixElementEnumerator> executionAliasEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(executionAliasQuery.c_str(), &executionAliasEnum));
            RETURN_IF_FAILED(executionAliasEnum->GetHasCurrent(&hc_executionAlias));

            while (hc_executionAlias)
            {
                ComPtr<IMsixElement> executionAliasElement;
                RETURN_IF_FAILED(executionAliasEnum->GetCurrent(&executionAliasElement));

                //alias
                Text<wchar_t> alias;
                RETURN_IF_FAILED(executionAliasElement->GetAttributeValue(executionAliasName.c_str(), &alias));
                m_appExecutionAliases.push_back(alias.Get());

                RETURN_IF_FAILED(executionAliasEnum->MoveNext(&hc_executionAlias));
            }
        }

        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }
    return S_OK;
}

HRESULT AppExecutionAlias::ProcessAliasForAdd(std::wstring & aliasName)
{
    RegistryKey appPathsKey;
    RETURN_IF_FAILED(appPathsKey.Open(HKEY_LOCAL_MACHINE, appPathsRegKeyName.c_str(), KEY_READ | KEY_WRITE));

    RegistryKey aliasKey;
    RETURN_IF_FAILED(appPathsKey.CreateSubKey(aliasName.c_str(), KEY_READ | KEY_WRITE, &aliasKey));

    //delete aliasKey if exists

    RETURN_IF_FAILED(aliasKey.SetStringValue(L"", m_msixRequest->GetPackageInfo()->GetResolvedExecutableFilePath()));

    std::wstring executableDirectoryPath;
    std::wstring executableFilePath = m_msixRequest->GetPackageInfo()->GetResolvedExecutableFilePath();
    size_t lastSlashPostion = executableFilePath.rfind('\\');
    if (std::wstring::npos != lastSlashPostion)
    {
        executableDirectoryPath = executableFilePath.substr(0, lastSlashPostion);
    }

    RETURN_IF_FAILED(aliasKey.SetStringValue(L"Path", executableDirectoryPath));

    std::wstring aliasDirectory = FilePathMappings::GetInstance().GetMsixCoreDirectory() + m_msixRequest->GetPackageInfo()->GetPackageFullName();
    std::wstring aliasFile = aliasDirectory + L"\\" + aliasName;

    HANDLE aliasFileHandle = INVALID_HANDLE_VALUE;
    aliasFileHandle = CreateFile(aliasFile.c_str(), GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (aliasFileHandle == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    BOOL result = CreateHardLink(executableFilePath.c_str(), aliasFile.c_str(), 0);
    if (!result)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    //can do this only once to add to the registry
    //SetEnvironmentVariable(L"Path", aliasDirectory.c_str());

    return S_OK;
}

HRESULT AppExecutionAlias::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<AppExecutionAlias > localInstance(new AppExecutionAlias(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
