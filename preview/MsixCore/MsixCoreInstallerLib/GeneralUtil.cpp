#include "GeneralUtil.hpp"
#include "MsixTraceLoggingProvider.hpp"
#include <string>
#include <codecvt>
#include <iostream>
#include <sddl.h>
#include <memory>
#pragma warning(disable : 4996)
namespace MsixCoreLib
{
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

    std::wstring ConvertVersionToString(UINT64 version)
    {
        return std::to_wstring((version >> 0x30) & 0xFFFF) + L"."
            + std::to_wstring((version >> 0x20) & 0xFFFF) + L"."
            + std::to_wstring((version >> 0x10) & 0xFFFF) + L"."
            + std::to_wstring((version) & 0xFFFF);
    }

    std::wstring GetFamilyNameFromFullName(const std::wstring& fullName)
    {
        return fullName.substr(0, fullName.find(L"_")) + fullName.substr(fullName.find_last_of(L"_"));
    }

    bool CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right)
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

    HRESULT GetCurrentUserSidString(std::wstring & userSidString)
    {
        HANDLE token;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        DWORD tokenSize = 0;
        GetTokenInformation(token, TokenUser, nullptr /*token buffer */, tokenSize, &tokenSize);
        DWORD lastError = GetLastError();
        if (lastError != ERROR_INSUFFICIENT_BUFFER)
        {
            return HRESULT_FROM_WIN32(lastError);
        }

        std::unique_ptr<BYTE[]> tokenBuffer(new BYTE[tokenSize]);
        if (!tokenBuffer)
        {
            return E_OUTOFMEMORY;
        }

        if (!GetTokenInformation(token, TokenUser, tokenBuffer.get(), tokenSize, &tokenSize))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        auto pTokenUser = reinterpret_cast<PTOKEN_USER>(tokenBuffer.get());

        PWSTR stringSid = nullptr;
        if (!ConvertSidToStringSid(pTokenUser->User.Sid, &stringSid))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        userSidString = stringSid;
        LocalFree(stringSid);

        return S_OK;
    }

    BOOL IsWindows10RS3OrLater()
    {
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 16299;

        DWORDLONG dwlConditionMask = 0;
        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

        return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
    }

    bool EndsWith(std::wstring const &fullString, std::wstring const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }

    bool IsPackageFile(std::wstring const& packageFilePath)
    {
        return EndsWith(packageFilePath, L".appx") || EndsWith(packageFilePath, L".msix");
    }

    bool IsBundleFile(std::wstring const& bundleFilePath)
    {
        return EndsWith(bundleFilePath, L".appxbundle") || EndsWith(bundleFilePath, L".msixbundle");
    }

}