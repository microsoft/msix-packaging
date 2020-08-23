#pragma once
#include <string>

namespace MsixCoreLib
{
    HRESULT CreateAndAddToCIM(
        _In_ std::wstring cimPath,
        _In_ std::wstring sourcePath,
        _In_ std::wstring rootDirectory);

    HRESULT MountCIM(
        std::wstring cimFilePath,
        GUID volumeId);

    HRESULT UnmountCIM(
        GUID volumeId);

}



