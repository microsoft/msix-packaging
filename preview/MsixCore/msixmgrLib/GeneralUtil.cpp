#include "GeneralUtil.hpp"
#include "MsixTraceLoggingProvider.hpp"
#include <string>
#include <codecvt>
#include <iostream>
#include <sddl.h>
#include <memory>
#include <filesystem>
#include <fstream>

// MSIXWindows.hpp defines NOMINMAX and undefines min and max because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. We can't use namespace std because c++17 defines std::byte, which conflicts with ::byte
#define max std::max
#define min std::min
#include <GdiPlus.h>

#pragma warning(disable : 4996)

using namespace std;

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

    /// Creates an .ico file next to the logo path which is .png by simply prepending the ICO header.
    HRESULT ConvertLogoToIcon(std::wstring logoPath, std::wstring & iconPath)
    {
        experimental::filesystem::path path(logoPath);
        iconPath = path.replace_extension(L".ico");

        bool fileExists = false;
        RETURN_IF_FAILED(FileExists(iconPath, fileExists));
        if (fileExists)
        {
            return S_OK;
        }

        uintmax_t size = experimental::filesystem::file_size(logoPath);
        ifstream input(logoPath, std::ios::binary);
        ofstream output(iconPath, std::ios::binary);

        Gdiplus::Image image(logoPath.c_str());

        //See https://en.wikipedia.org/wiki/ICO_(file_format)
        BYTE iconHeader[] = {
            0,0,                                    // reserved
            1,0,                                    // 1 for .ico icon
            1,0,                                    // 1 image in file
            static_cast<BYTE>(image.GetWidth()),    // width
            static_cast<BYTE>(image.GetHeight()),   // height
            0,                                      // colors in color palette
            0,                                      // reserved
            1,0,                                    // color planes
            32,0,                                   // bits per pixel
            0,0,0,0,                                // size of image in bytes
            0,0,0,0 };                              // offset from start of file of actual image

        // fill in size
        iconHeader[14] = static_cast<BYTE>(size % 256);
        iconHeader[15] = static_cast<BYTE>(size / 256);

        // fill in offset from start of file, which is the size of this header
        iconHeader[18] = static_cast<BYTE>(ARRAYSIZE(iconHeader));

        for (int i = 0; i < ARRAYSIZE(iconHeader); i++)
        {
            output << iconHeader[i];
        }
        output << input.rdbuf();
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

    bool IsPackageFile(std::wstring const& path)
    {
        return EndsWith(path, L".appx") || EndsWith(path, L".msix");
    }

    bool IsBundleFile(std::wstring const& path)
    {
        return EndsWith(path, L".appxbundle") || EndsWith(path, L".msixbundle");
    }

    /// Replaces all oldchars in input with newchar
    ///
    /// @param input   - The input string that contains the characters to be changed
    /// @param oldchar - Old character that are to be replaced
    /// @param newchar - New character that replaces oldchar
    void replace(std::wstring& input, const wchar_t oldchar, const wchar_t newchar)
    {
        std::size_t found = input.find_first_of(oldchar);
        while (found != std::string::npos)
        {
            input[found] = newchar;
            found = input.find_first_of(oldchar, found + 1);
        }
    }

    /// Makes a directory, including all parent directories based on the inputted filepath
    ///
    /// @param utf16Path - The filepath to create a directory in utf16
    int mkdirp(std::wstring& utf16Path)
    {
        replace(utf16Path, L'/', L'\\');
        for (std::size_t i = 3; i < utf16Path.size(); i++) // 3 skips past c:
        {
            if (utf16Path[i] == L'\0')
            {
                break;
            }
            else if (utf16Path[i] == L'\\')
            {
                // Temporarily set string to terminate at the '\' character
                // to obtain name of the subdirectory to create
                utf16Path[i] = L'\0';

                if (!CreateDirectoryW(utf16Path.c_str(), nullptr))
                {
                    int lastError = static_cast<int>(GetLastError());

                    // It is normal for CreateDirectory to fail if the subdirectory
                    // already exists.  Other errors should not be ignored.
                    if (lastError != ERROR_ALREADY_EXISTS)
                    {
                        return lastError;
                    }
                }
                // Restore original string
                utf16Path[i] = L'\\';
            }
        }
        return 0;
    }

}