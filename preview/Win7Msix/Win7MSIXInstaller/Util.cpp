#include "Util.hpp"
#include <string>
#include <codecvt>
#include <iostream>

std::string utf16_to_utf8(const std::wstring& utf16string)
{
	auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
	std::string result(converted.begin(), converted.end());
	return result;
}

std::wstring utf8_to_utf16(const std::string& utf8string)
{
	// see https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
	auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
	std::wstring result(converted.begin(), converted.end());
	return result;
}

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

bool CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right)
{
    return (_wcsicmp(left.c_str(), right.c_str()) == 0);
}