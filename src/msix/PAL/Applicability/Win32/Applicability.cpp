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
#include <iterator>
#include <algorithm>

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

    // This is not an exhaustive list and it doesn't include LIP languages
    // but covers the most common Win7 MUIs out there.
    static const MuiToBcp47Entry muiToBcp47List[] = {
        MuiToBcp47Entry(L"ar-SA", u8"ar-SA"),
        MuiToBcp47Entry(L"bg-BG", u8"bg"),
        MuiToBcp47Entry(L"cs-CZ", u8"cs"),
        MuiToBcp47Entry(L"da-DK", u8"da"),
        MuiToBcp47Entry(L"de-DE", u8"de-DE"),
        MuiToBcp47Entry(L"el-GR", u8"el"),
        MuiToBcp47Entry(L"en-US", u8"en-US"),
        MuiToBcp47Entry(L"es-ES", u8"es-ES"),
        MuiToBcp47Entry(L"et-EE", u8"et"),
        MuiToBcp47Entry(L"fi-FI", u8"fi"),
        MuiToBcp47Entry(L"fr-FR", u8"fr-FR"),
        MuiToBcp47Entry(L"he-IL", u8"he"),
        MuiToBcp47Entry(L"hi-IN", u8"hi"),
        MuiToBcp47Entry(L"hr-HR", u8"hr-HR"),
        MuiToBcp47Entry(L"hu-HU", u8"hu"),
        MuiToBcp47Entry(L"it-IT", u8"it-IT"),
        MuiToBcp47Entry(L"ja-JP", u8"ja"),
        MuiToBcp47Entry(L"ko-KR", u8"ko"),
        MuiToBcp47Entry(L"lt-LT", u8"lt"),
        MuiToBcp47Entry(L"lv-LV", u8"lv"),
        MuiToBcp47Entry(L"nb-NO", u8"nb"),
        MuiToBcp47Entry(L"nl-NL", u8"nl-NL"),
        MuiToBcp47Entry(L"pl-PL", u8"pl"),
        MuiToBcp47Entry(L"ps-PS", u8"ps-PS"),
        MuiToBcp47Entry(L"pt-BR", u8"pt-BR"),
        MuiToBcp47Entry(L"pt-PT", u8"pt-PT"),
        MuiToBcp47Entry(L"ro-RO", u8"ro-RO"),
        MuiToBcp47Entry(L"ru-RU", u8"ru"),
        MuiToBcp47Entry(L"sk-SK", u8"sk"),
        MuiToBcp47Entry(L"sl-SI", u8"sl"),
        MuiToBcp47Entry(L"sr-Latn-CS", u8"sr-Latn-CS"),
        MuiToBcp47Entry(L"sv-SE", u8"sv-SE"),
        MuiToBcp47Entry(L"th-TH", u8"th"),
        MuiToBcp47Entry(L"tr-TR", u8"tr"),
        MuiToBcp47Entry(L"uk-UA", u8"uk"),
        MuiToBcp47Entry(L"zh-CN", u8"zh-Hans-CN"),
        MuiToBcp47Entry(L"zh-HK", u8"zh-Hant-HK"),
        MuiToBcp47Entry(L"zh-TW", u8"zh-Hant-TW"),
    };

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
            ThrowHrIfFalse(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numOfLangs, const_cast<PZZWSTR>(languagesWin7.data()), &size),
                "Failed GetUserPreferredUILanguages");

            size_t position = 0;
            ULONG processedTags = 0;
            auto delimiter = L'\0';
            auto found = languagesWin7.find(delimiter);
            while(found != std::string::npos && processedTags < numOfLangs)
            {
                auto muiTag = languagesWin7.substr(position, found - position);
                const auto& tag = std::find(std::begin(muiToBcp47List), std::end(muiToBcp47List), MuiToBcp47Entry(muiTag.c_str(), nullptr));
                if (tag == std::end(muiToBcp47List))
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
