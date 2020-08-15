#pragma once
#include <string>

namespace MsixCoreLib
{
    HRESULT CreateAndAddToCIM(
        _In_ std::wstring cimPath,
        _In_ std::wstring sourcePath,
        _In_ std::wstring rootDirectory);

}



