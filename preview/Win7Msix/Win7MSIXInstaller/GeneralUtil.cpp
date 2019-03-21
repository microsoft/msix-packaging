#include "GeneralUtil.hpp"
#include <string>
#include <codecvt>
#include <iostream>

/// Converts a wstring from utf16 to utf8
///
/// @param utf16string - A utf16 wstring
/// @return utf8 string
std::string utf16_to_utf8(const std::wstring& utf16string)
{
	auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
	std::string result(converted.begin(), converted.end());
	return result;
}

/// Converts a string from utf8 to utf16
///
/// @param utf8string - A utf8 string
/// @return utf16 string
std::wstring utf8_to_utf16(const std::string& utf8string)
{
	// see https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
	auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
	std::wstring result(converted.begin(), converted.end());
	return result;
}

/// Helper to convert version number to a version string of the form a.b.c.d
///
/// @param version - version number
/// @return a.b.c.d string representation of version
std::wstring ConvertVersionToString(UINT64 version)
{
    return std::to_wstring((version >> 0x30) & 0xFFFF) + L"."
        + std::to_wstring((version >> 0x20) & 0xFFFF) + L"."
        + std::to_wstring((version >> 0x10) & 0xFFFF) + L"."
        + std::to_wstring((version) & 0xFFFF);
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

HRESULT GetAttributeValueFromElement(IMsixElement * element, const std::wstring attributeName, std::wstring & attributeValue)
{
    Text<wchar_t> textValue;
    RETURN_IF_FAILED(element->GetAttributeValue(attributeName.c_str(), &textValue));
    if (textValue.Get() != nullptr)
    {
        attributeValue = textValue.Get();
    }

    return S_OK;
}

std::wstring GuidFromManifestId(std::wstring id)
{
    return L"{" + id + L"}";
}

HRESULT FileExists(std::wstring file, _Out_ bool &exists)
{
    DWORD fileAttributes = GetFileAttributesW(file.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        DWORD lastError = GetLastError();
        if ((lastError == ERROR_FILE_NOT_FOUND) || (lastError == ERROR_PATH_NOT_FOUND))
        {
            exists = false;
        }
        else
        {
            return HRESULT_FROM_WIN32(lastError);
        }
    }
    else
    {
        exists = true;
    }
    return S_OK;
}