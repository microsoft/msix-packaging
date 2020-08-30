#pragma once
#include <string>

namespace MsixCoreLib
{
    HRESULT CreateAndMountVHD(
        _In_ std::wstring vhdFilePath,
        _In_ ULONGLONG sizeMBs,
        _Inout_ std::wstring& driveLetter);

    HRESULT UnmountVHD(
        _In_ std::wstring vhdFilePath);

    HRESULT MountVHD(
        _In_ std::wstring vhdFilePath,
        _Inout_ std::wstring& driveLetter);
}