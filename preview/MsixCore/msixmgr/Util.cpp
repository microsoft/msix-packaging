#include "Util.hpp"
#include <string>
#include <codecvt>
#include <iostream>

std::wstring GetStringResource(UINT resourceId)
{
    HMODULE instance = GetModuleHandle(nullptr);

    WCHAR buffer[MAX_PATH] = L"";
    int loadStringRet = LoadStringW(instance, resourceId, buffer, ARRAYSIZE(buffer));
    if (loadStringRet <= 0)
    {
        return std::wstring(L"Failed to load string resource");
    }

    std::wstring stringResource(buffer);

    return stringResource;
}