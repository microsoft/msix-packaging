#pragma once

#include "GeneralUtil.hpp"
#include "MsixResponse.hpp"

namespace MsixCoreLib
{
    /// These are functions that redirect on windows 10 using the winRT Windows::Management::Deployment::PackageManager APIs
    /// These will attempt to delayload apisets only available on windows 10, so caller should check that the OS is windows 10 before calling.
    namespace Windows10Redirector
    {
        HRESULT AddPackageWithProgress(const std::wstring & packageFilePath, std::shared_ptr<MsixResponse>& msixResponse);
        HRESULT AddPackage(const std::wstring & packageFilePath);
        HRESULT RemovePackage(const std::wstring & packageFullName);
    };
}