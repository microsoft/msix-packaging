#include <windows.h>
#include <pathcch.h>
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
                //appExecutionAlias.key = alias.Get();

                //package family name

                //aumid

                //app full executable path
            }
        }
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

    std::wstring executableDirectoryPath = m_msixRequest->GetPackageInfo()->GetResolvedExecutableFilePath();
    UINT32 executableDirectoryPathLength = executableDirectoryPath.size();
    RETURN_IF_FAILED(PathCchRemoveFileSpec((PWSTR)executableDirectoryPath.c_str(), executableDirectoryPathLength));

    RETURN_IF_FAILED(aliasKey.SetStringValue(L"Path", executableDirectoryPath));

    return S_OK;
}

HRESULT AppExecutionAlias::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<AppExecutionAlias > localInstance(new AppExecutionAlias(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
