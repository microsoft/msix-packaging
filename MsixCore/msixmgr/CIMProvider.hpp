#pragma once
#include <string>

namespace MsixCoreLib
{
    HRESULT CreateAndAddToCIM(
        _In_ std::wstring cimPath,
        _In_ std::wstring sourcePath,
        _In_ std::wstring rootDirectory);

    HRESULT MountCIM(
        _In_ std::wstring cimFilePath,
        _Out_ std::wstring& volumeId);

    HRESULT UnmountCIM(
        _In_opt_ std::wstring cimFilePath,
        _In_opt_ std::wstring volumeIdString);
}



