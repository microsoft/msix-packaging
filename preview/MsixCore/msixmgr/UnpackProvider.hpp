#pragma once
#include <string>
#include <vector>
#include "MSIXWindows.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"

namespace MsixCoreLib
{
    HRESULT UnpackPackage(
        _In_ std::wstring packageFilePath,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs);

    HRESULT UnpackBundle(
        _In_ std::wstring packageFilePath,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs);

    HRESULT OutputPackageDependencies(
        _In_ IAppxManifestReader* manifestReader,
        _In_ LPWSTR packageFullName);

}