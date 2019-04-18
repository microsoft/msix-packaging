#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ValidateTargetDeviceFamily.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include <VersionHelpers.h>
#include <sstream>

const PCWSTR ValidateTargetDeviceFamily::HandlerName = L"ValidateRequest";

HRESULT ValidateTargetDeviceFamily::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(ParseTargetDeviceFamilyFromPackage());
    if (m_targetDeviceFamilyName == L"MSIXCore.Desktop" && IsManifestVersionCompatilbleWithOS())
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Validation complete",
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
    }
    else
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "TDF or OS version did not match",
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
    }
    return S_OK;
}

HRESULT ValidateTargetDeviceFamily::ParseTargetDeviceFamilyFromPackage()
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
        return HRESULT_FROM_WIN32(ERROR_INSTALL_REJECTED);
    }

    ComPtr<IMsixElement> dependencyElement;
    RETURN_IF_FAILED(dependencyEnum->GetCurrent(&dependencyElement));

    Text<wchar_t> targetDeviceFamilyName;
    RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"Name", &targetDeviceFamilyName));
    m_targetDeviceFamilyName = targetDeviceFamilyName.Get();

    Text<wchar_t> minVersion;
    RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"MinVersion", &minVersion));
    m_minVersion = minVersion.Get();

    //Major version
    size_t start = 0;
    size_t end = m_minVersion.find_first_of(L'.');
    m_majorVersion = m_minVersion.substr(start, end - start);

    //Minor version
    m_minVersion.replace(start, end - start + 1, L"");
    end = m_minVersion.find_first_of(L'.');
    m_minorVersion = m_minVersion.substr(start, end - start);

    //Build number
    m_minVersion.replace(start, end - start + 1, L"");
    end = m_minVersion.find_first_of(L'.');
    m_buildNumber = m_minVersion.substr(start, end - start);

    return S_OK;
}

bool ValidateTargetDeviceFamily::IsManifestVersionCompatilbleWithOS()
{
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;
    int op = VER_GREATER_EQUAL;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = std::stoi(m_majorVersion.c_str());
    osvi.dwMinorVersion = std::stoi(m_minorVersion.c_str());
    osvi.dwBuildNumber = std::stoi(m_buildNumber.c_str());

    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);

    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
}

HRESULT ValidateTargetDeviceFamily::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ValidateTargetDeviceFamily> localInstance(new ValidateTargetDeviceFamily(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
