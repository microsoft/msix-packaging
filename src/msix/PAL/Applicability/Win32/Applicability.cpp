//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include <windows.h>
#include <VersionHelpers.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <windows.system.userprofile.h>

#include "Applicability.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "UnicodeConversion.hpp"

#include <vector>
#include <map>

using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::System::UserProfile;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Foundation;

namespace MSIX {

    struct MuiToBcp47Entry
    {
        const wchar_t* mui;
        const char*    bcp47;

        MuiToBcp47Entry(const wchar_t* m, const char* b) : mui(m), bcp47(b) {}

        inline bool operator==(const MuiToBcp47Entry& rhs) const {
            return 0 == wcscmp(mui, rhs.mui);
        }
    };
    typedef std::vector<MuiToBcp47Entry> MuiToBcp47Manager;

    // This is not an exhaustive list and it doesn't include LIP languages
    // but covers the most common Win7 MUIs out there.
    static const MuiToBcp47Manager muiToBcp47List[] = {
    {
        MuiToBcp47Entry(L"ar-SA", "ar-SA"),
        MuiToBcp47Entry(L"bg-BG", "bg"),
        MuiToBcp47Entry(L"cs-CZ", "cs"),
        MuiToBcp47Entry(L"da-DK", "da"),
        MuiToBcp47Entry(L"de-DE", "de-DE"),
        MuiToBcp47Entry(L"el-GR", "el"),
        MuiToBcp47Entry(L"en-US", "en-US"),
        MuiToBcp47Entry(L"es-ES", "es-ES"),
        MuiToBcp47Entry(L"et-EE", "et"),
        MuiToBcp47Entry(L"fi-FI", "fi"),
        MuiToBcp47Entry(L"fr-FR", "fr-FR"),
        MuiToBcp47Entry(L"he-IL", "he"),
        MuiToBcp47Entry(L"hi-IN", "hi"),
        MuiToBcp47Entry(L"hr-HR", "hr-HR"),
        MuiToBcp47Entry(L"hu-HU", "hu"),
        MuiToBcp47Entry(L"it-IT", "it-IT"),
        MuiToBcp47Entry(L"ja-JP", "ja"),
        MuiToBcp47Entry(L"ko-KR", "ko"),
        MuiToBcp47Entry(L"lt-LT", "lt"),
        MuiToBcp47Entry(L"lv-LV", "lv"),
        MuiToBcp47Entry(L"nb-NO", "nb"),
        MuiToBcp47Entry(L"nl-NL", "nl-NL"),
        MuiToBcp47Entry(L"pl-PL", "pl"),
        MuiToBcp47Entry(L"ps-PS", "ps-PS"),
        MuiToBcp47Entry(L"pt-BR", "pt-BR"),
        MuiToBcp47Entry(L"pt-PT", "pt-PT"),
        MuiToBcp47Entry(L"ro-RO", "ro-RO"),
        MuiToBcp47Entry(L"ru-RU", "ru"),
        MuiToBcp47Entry(L"sk-SK", "sk"),
        MuiToBcp47Entry(L"sl-SI", "sl"),
        MuiToBcp47Entry(L"sr-Latn-CS", "sr-Latn-CS"),
        MuiToBcp47Entry(L"sv-SE", "sv-SE"),
        MuiToBcp47Entry(L"th-TH", "th"),
        MuiToBcp47Entry(L"tr-TR", "tr"),
        MuiToBcp47Entry(L"uk-UA", "uk"),
        MuiToBcp47Entry(L"zh-CN", "zh-Hans-CN"),
        MuiToBcp47Entry(L"zh-HK", "zh-Hant-HK"),
        MuiToBcp47Entry(L"zh-TW", "zh-Hant-TW"),
    },
    };

    #define ThrowHrIfFalse(a, m)                                                                               \
    {   BOOL _result = a;                                                                                      \
        if (!_result)                                                                                          \
        {   MSIX::RaiseException<MSIX::Exception> (__LINE__, __FILE__, m, HRESULT_FROM_WIN32(GetLastError())); \
        }                                                                                                      \
    }

    MSIX_PLATFORM Applicability::GetPlatform()
    {
        if (IsWindows10OrGreater())
        {
            return MSIX_PLATFORM_WINDOWS10;
        }
        else if (IsWindows8Point1OrGreater())
        {
            return MSIX_PLATFORM_WINDOWS8POINT1;
        }
        else if (IsWindows8OrGreater())
        {
            return MSIX_PLATFORM_WINDOWS8;
        }
        else if (IsWindows7SP1OrGreater() || IsWindows7OrGreater())
        {
            return MSIX_PLATFORM_WINDOWS7;
        }
        UNEXPECTED;
    }

    std::vector<std::string> Applicability::GetLanguages()
    {
        std::vector<std::string> result;

        if (MSIX_PLATFORM_WINDOWS7 == GetPlatform())
        {
            ULONG numOfLangs = 0;
            DWORD size = 0;
            std::wstring languagesWin7;
            ThrowHrIfFalse(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numOfLangs, nullptr, &size),
                "Failed GetUserPreferredUILanguages");
            languagesWin7.resize(size);
            ThrowHrIfFalse(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numOfLangs, &languagesWin7.front(), &size),
                "Failed GetUserPreferredUILanguages");

            size_t position = 0;
            ULONG processedTags = 0;
            wchar_t delimiter = '\0';
            auto found = languagesWin7.find(delimiter);
            while(found != std::string::npos && processedTags < numOfLangs)
            {
                auto muiTag = languagesWin7.substr(position, found - position);
                const auto& tag = std::find(muiToBcp47List[0].begin(), muiToBcp47List[0].end(), MuiToBcp47Entry(muiTag.c_str(), nullptr));
                if (tag == muiToBcp47List[0].end())
                {   // Is not well known, luckily the tag will be the same (probably not) :)
                    result.push_back(utf16_to_utf8(muiTag));
                }
                else
                {
                    result.push_back((*tag).bcp47);  
                }
                position = found+1;
                processedTags++;
                found = languagesWin7.find(delimiter, position);
            }
        }
        else
        {
            RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
            ThrowHrIfFailed(initialize);
            ComPtr<IGlobalizationPreferencesStatics> globalizationPreferences;
            ComPtr<IVectorView<HSTRING>> languages;
            ThrowHrIfFailed(GetActivationFactory(HStringReference(
                RuntimeClass_Windows_System_UserProfile_GlobalizationPreferences).Get(),&globalizationPreferences));
            ThrowHrIfFailed(globalizationPreferences->get_Languages(&languages));
            UINT numOfLanguages = 0;
            ThrowHrIfFailed(languages->get_Size(&numOfLanguages));
            for (UINT i = 0; i < numOfLanguages; i++)
            {
                HString hstring;
                ThrowHrIfFailed(languages->GetAt(i, hstring.GetAddressOf()));
                result.push_back(utf16_to_utf8(hstring.GetRawBuffer(nullptr)));
            }
        }
        return result;
    }
}
