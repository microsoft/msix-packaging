#pragma once
#include <string>
#include <vector>
#include "MSIXWindows.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"

namespace MsixCoreLib
{
    HRESULT Unpack(
        _In_ std::wstring source,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs,
        _In_ bool validateSignature,
        _Inout_ std::vector<std::wstring> &skippedFiles,
        _Inout_ std::vector<std::wstring> &failedPackages,
        _Inout_ std::vector<HRESULT> &failedPackagesErrors);

    HRESULT UnpackPackagesFromDirectory(
        _In_ std::wstring source,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs,
        _In_ bool validateSignature,
        _Inout_ std::vector<std::wstring> &skippedFiles,
        _Inout_ std::vector<std::wstring> &failedPackages,
        _Inout_ std::vector<HRESULT> &failedPackagesErrors);

    HRESULT UnpackPackageOrBundle(
        _In_ std::wstring source,
        _In_ std::wstring destination,
        _In_ bool isApplyACLs,
        _In_ bool validateSignature);

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