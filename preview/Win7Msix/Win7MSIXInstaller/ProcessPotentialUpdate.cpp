#include "ProcessPotentialUpdate.hpp"
#include <filesystem>

const PCWSTR ProcessPotentialUpdate::HandlerName = L"ProcessPotentialUpdate";

HRESULT ProcessPotentialUpdate::ExecuteForAddRequest()
{
    std::wstring currentPackageFamilyName = GetFamilyNameFromFullName(m_msixRequest->GetPackageInfo()->GetPackageFullName());
    
    for (auto& p : std::experimental::filesystem::directory_iterator(m_msixRequest->GetFilePathMappings()->GetMsix7Directory()))
    {
        std::wstring installedPackageFamilyName = GetFamilyNameFromFullName(p.path().filename());
        if (CaseInsensitiveEquals(currentPackageFamilyName, installedPackageFamilyName)
            && !CaseInsensitiveEquals(m_msixRequest->GetPackageInfo()->GetPackageFullName(), p.path().filename()))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Found an update to an existing package, removing package",
                TraceLoggingValue(p.path().filename().c_str(), "PackageToBeRemoved"));

            RETURN_IF_FAILED(RemovePackage(p.path().filename()));
            return S_OK;
        }
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Not an update, nothing to do.");
    return S_OK;
}

HRESULT ProcessPotentialUpdate::RemovePackage(std::wstring packageFullName)
{
    AutoPtr<MsixRequest> localRequest;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Remove, Flags::NoFlags, std::wstring(), packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &localRequest));

    const HRESULT hrProcessRequest = localRequest->ProcessRequest();
    if (FAILED(hrProcessRequest))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Failed to remove package",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrProcessRequest, "HR"));
    }

    return S_OK;
}

HRESULT ProcessPotentialUpdate::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ProcessPotentialUpdate> localInstance(new ProcessPotentialUpdate(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}