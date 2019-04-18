#include "GeneralUtil.hpp"
#include <string>
#include <codecvt>
#include <iostream>
using namespace Win7MsixInstallerLib;

TRACELOGGING_DEFINE_PROVIDER(
	g_MsixTraceLoggingProvider,
	"MsixTraceLoggingProvider",
	(0x033321d3, 0xd599, 0x48e0, 0x86, 0x8d, 0xc5, 0x9f, 0x15, 0x90, 0x16, 0x37));

/// Converts a wstring from utf16 to utf8
///
/// @param utf16string - A utf16 wstring
/// @return utf8 string
std::string Win7MsixInstallerLib_utf16_to_utf8(const std::wstring& utf16string)
{
	auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
	std::string result(converted.begin(), converted.end());
	return result;
}

/// Helper to convert version number to a version string of the form a.b.c.d
///
/// @param version - version number
/// @return a.b.c.d string representation of version
std::wstring Win7MsixInstallerLib_ConvertVersionToString(UINT64 version)
{
    return std::to_wstring((version >> 0x30) & 0xFFFF) + L"."
        + std::to_wstring((version >> 0x20) & 0xFFFF) + L"."
        + std::to_wstring((version >> 0x10) & 0xFFFF) + L"."
        + std::to_wstring((version) & 0xFFFF);
}

std::wstring Win7MsixInstallerLib_GetFamilyNameFromFullName(const std::wstring& fullName)
{
    return fullName.substr(0, fullName.find(L"_")) + fullName.substr(fullName.find_last_of(L"_"));
}

bool Win7MsixInstallerLib_CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right)
{
    return (_wcsicmp(left.c_str(), right.c_str()) == 0);
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