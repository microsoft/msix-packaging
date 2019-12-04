#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ValidateArchitecture.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <VersionHelpers.h>
#include "Constants.hpp"

using namespace MsixCoreLib;

const PCWSTR ValidateArchitecture::HandlerName = L"ValidateArchitecture";

HRESULT ValidateArchitecture::ExecuteForAddRequest()
{
    if (!IsArchitectureCompatibleWithOS())
    {
        return HRESULT_FROM_WIN32(ERROR_INSTALL_WRONG_PROCESSOR_ARCHITECTURE);
    }

    return S_OK;
}

bool ValidateArchitecture::IsArchitectureCompatibleWithOS()
{
    APPX_PACKAGE_ARCHITECTURE packageArchitecture = m_msixRequest->GetPackageInfo()->GetArchitecture();

    if (packageArchitecture == APPX_PACKAGE_ARCHITECTURE_NEUTRAL)
    {
        return true;
    }

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    APPX_PACKAGE_ARCHITECTURE machineArchitecture = static_cast<APPX_PACKAGE_ARCHITECTURE>(systemInfo.wProcessorArchitecture);

    if (packageArchitecture == machineArchitecture)
    {
        return true;
    }

    // Allow x86 packages to install on x64 machines
    if (machineArchitecture == APPX_PACKAGE_ARCHITECTURE_X64 &&
        packageArchitecture == APPX_PACKAGE_ARCHITECTURE_X86)
    {
        return true;
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Incompatible Architecture",
        TraceLoggingValue(static_cast<DWORD>(packageArchitecture), "packageArchitecture"),
        TraceLoggingValue(static_cast<DWORD>(machineArchitecture), "machineArchitecture"));
    return false;
}

HRESULT ValidateArchitecture::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ValidateArchitecture> localInstance(new ValidateArchitecture(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
