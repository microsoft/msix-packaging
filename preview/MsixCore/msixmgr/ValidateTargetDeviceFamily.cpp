#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ValidateTargetDeviceFamily.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <VersionHelpers.h>
#include "Constants.hpp"

using namespace MsixCoreLib;

const PCWSTR ValidateTargetDeviceFamily::HandlerName = L"ValidateTargetDeviceFamily";

HRESULT ValidateTargetDeviceFamily::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(ParseAndValidateTargetDeviceFamilyFromPackage());
    return S_OK;
}

HRESULT ValidateTargetDeviceFamily::ParseAndValidateTargetDeviceFamilyFromPackage()
{
    auto packageInfo = m_msixRequest->GetPackageInfo();

    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(packageInfo->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    /// Obtain the TargetDeviceFamily tag info
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

    while (hc)
    {
        ComPtr<IMsixElement> dependencyElement;
        RETURN_IF_FAILED(dependencyEnum->GetCurrent(&dependencyElement));

        Text<wchar_t> targetDeviceFamilyName;
        RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"Name", &targetDeviceFamilyName));
        m_targetDeviceFamilyName = targetDeviceFamilyName.Get();

        Text<wchar_t> minVersion;
        RETURN_IF_FAILED(dependencyElement->GetAttributeValue(L"MinVersion", &minVersion));
        std::wstring manifestMinVersion = minVersion.Get();

        /// Major version
        size_t start = 0;
        size_t end = manifestMinVersion.find_first_of(L'.');
        m_majorVersion = manifestMinVersion.substr(start, end - start);

        /// Minor version
        manifestMinVersion.replace(start, end - start + 1, L"");
        end = manifestMinVersion.find_first_of(L'.');
        m_minorVersion = manifestMinVersion.substr(start, end - start);

        /// Build number
        manifestMinVersion.replace(start, end - start + 1, L"");
        end = manifestMinVersion.find_first_of(L'.');
        m_buildNumber = manifestMinVersion.substr(start, end - start);

        /// Return if any one of the target device families are compatible with OS
        if (IsTargetDeviceFamilyNameCompatibleWithOS() && IsManifestVersionCompatibleWithOS())
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Target device family name and manifest min version are compatible with OS",
                TraceLoggingLevel(WINEVENT_LEVEL_INFO),
                TraceLoggingValue(m_targetDeviceFamilyName.c_str(), "TargetDeviceFamilyName"),
                TraceLoggingValue(minVersion.Get(), "ManifestMinVersion"));
            return S_OK;
        }
        else
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Target device family name and manifest min version are not compatible with OS",
                TraceLoggingLevel(WINEVENT_LEVEL_INFO),
                TraceLoggingValue(m_targetDeviceFamilyName.c_str(), "TargetDeviceFamilyName"),
                TraceLoggingValue(minVersion.Get(), "ManifestMinVersion"));
        }

        RETURN_IF_FAILED(dependencyEnum->MoveNext(&hc));
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Target device family name or manifest min version are not compatible with the OS",
        TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
    return HRESULT_FROM_WIN32(ERROR_INSTALL_PREREQUISITE_FAILED);
}

bool ValidateTargetDeviceFamily::IsTargetDeviceFamilyNameCompatibleWithOS()
{
    if (IsWindowsProductTypeServer()) /// Server OS
    {
        if(CaseInsensitiveEquals(m_targetDeviceFamilyName, serverTargetDeviceFamilyName) || CaseInsensitiveEquals(m_targetDeviceFamilyName, desktopTargetDeviceFamilyName))
        {
            return true;
        }
    }
    else if (IsWindowsProductTypeDesktop()) /// Desktop OS
    {
        if (CaseInsensitiveEquals(m_targetDeviceFamilyName, desktopTargetDeviceFamilyName))
        {
            return true;
        }
    }
    return false;
}

bool ValidateTargetDeviceFamily::IsManifestVersionCompatibleWithOS()
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

bool ValidateTargetDeviceFamily::IsWindowsProductTypeDesktop()
{
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    return VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
}

bool ValidateTargetDeviceFamily::IsWindowsProductTypeServer()
{
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.wProductType = VER_NT_SERVER;

    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    return VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
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
