#pragma once
#include <string>

namespace MsixCoreLib
{
    HRESULT CreateAndMountVHD(
        _In_ std::wstring vhdFilePath);

    HRESULT UnmountVHD(
        _In_ std::wstring vhdFilePath);

    HRESULT MountVHD(
        _In_ std::wstring vhdFilePath);
}



