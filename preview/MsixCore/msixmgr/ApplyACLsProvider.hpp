#pragma once
#include <string>
#include <vector>

namespace MsixCoreLib
{
    class ApplyACLsProvider
    {
    public:

        ApplyACLsProvider() {};
        ~ApplyACLsProvider() {};

        static HRESULT ApplyACLs(
            _In_ std::vector<std::wstring> packageFolders);

    };
}



