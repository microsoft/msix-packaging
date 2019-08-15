#pragma once
#include <string>
#include <vector>

namespace MsixCoreLib
{
    class UnpackProvider
    {
    public:

        UnpackProvider() {};
        ~UnpackProvider() {}

        static HRESULT UnpackPackage(
            _In_ std::wstring packageFilePath,
            _In_ std::wstring destination,
            _In_ bool isApplyACLs);

        static HRESULT UnpackBundle(
            _In_ std::wstring packageFilePath,
            _In_ std::wstring destination,
            _In_ bool isApplyACLs);

    };
}