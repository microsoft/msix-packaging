#pragma once
#include <string>
#include <vector>

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
}