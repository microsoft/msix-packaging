#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ValidateRequest.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include <VersionHelpers.h>

const PCWSTR ValidateRequest::HandlerName = L"ValidateRequest";

HRESULT ValidateRequest::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(ParseDependencyInfoFromPackage());
    RETURN_IF_FAILED(MatchTDFAndVersionToOS());
    return S_OK;
}

HRESULT ValidateRequest::ParseDependencyInfoFromPackage()
{
    PackageInfo* packageInfo = m_msixRequest->GetPackageInfo();

    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(packageInfo->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    // Obtain the TargetDeviceFamily tag info
    ComPtr<IMsixElementEnumerator> dependencyEnum;
    RETURN_IF_FAILED(element->GetElements(
        L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='TargetDeviceFamily']",
        &dependencyEnum));

    BOOL hc = FALSE;
    RETURN_IF_FAILED(dependencyEnum->GetHasCurrent(&hc));
    if (!hc)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "No Target device family Found",
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
        return E_NOT_SET;
    }

    ComPtr<IMsixElement> dependencyElement;
    RETURN_IF_FAILED(dependencyEnum->GetCurrent(&dependencyElement));

    Text<wchar_t> targetDeviceFamilyName;
    RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"Name", &targetDeviceFamilyName));
    m_targetDeviceFamilyName = targetDeviceFamilyName.Get();

    Text<wchar_t> minVersion;
    RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"MinVersion", &minVersion));
    m_minVersion = minVersion.Get();

    return S_OK;
}

HRESULT ValidateRequest::MatchTDFAndVersionToOS()
{
    //Check for win 10 < 1709 first
    // If not win 10, then check for Windows 8
    if (IsWindows8OrGreater()) 
    {
        RETURN_IF_FAILED((m_targetDeviceFamilyName != L"MSIXCore.Desktop" && m_minVersion != L"6.2.9200.0") ||
                         (m_targetDeviceFamilyName != L"MSIXCore.Desktop" && m_minVersion != L"6.3.9200.0") ||
                         (m_targetDeviceFamilyName != L"MSIXCore.Desktop" && m_minVersion != L"6.3.9600.0"));
    }
    else if (IsWindows7SP1OrGreater()) //Windows 7 SP1
    {
        RETURN_IF_FAILED(m_targetDeviceFamilyName != L"MSIXCore.Desktop" && m_minVersion != L"6.1.7601.0");
    }
    return S_OK;
}

HRESULT ValidateRequest::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ValidateRequest> localInstance(new ValidateRequest(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
