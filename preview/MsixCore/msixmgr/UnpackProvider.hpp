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
        _In_ bool isApplyACLs,
        _In_ bool validateSignature);

    HRESULT UnpackBundle(
        _In_ std::wstring packageFilePath,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs,
        _In_ bool validateSignature);

    HRESULT OutputPackageDependencies(
        _In_ IAppxManifestReader* manifestReader,
        _In_ LPWSTR packageFullName);

}