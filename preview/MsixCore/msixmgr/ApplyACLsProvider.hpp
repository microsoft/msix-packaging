#pragma once
#include <string>
#include <vector>

namespace MsixCoreLib
{
    HRESULT ApplyACLs(
        _In_ std::vector<std::wstring> packageFolders);
}



