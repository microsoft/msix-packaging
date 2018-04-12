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

#include <set>
#include <map>

using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::System::UserProfile;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Foundation;


namespace MSIX {

    // This is not an exhaustive list and it doesn't include LIP languages
    // but covers the most common Win7 MUIs out there.
    const std::map<std::wstring, std::string> muiToBcp47 {
        {L"ar-SA", "ar-SA"},
        {L"bg-BG", "bg"},
        {L"cs-CZ", "cs"},
        {L"da-DK", "da"},
        {L"de-DE", "de-DE"},
        {L"el-GR", "el"},
        {L"en-US", "en-US"},
        {L"es-ES", "es-ES"},
        {L"et-EE", "et"},
        {L"fi-FI", "fi"},
        {L"fr-FR", "fr-FR"},
        {L"he-IL", "he"},
        {L"hi-IN", "hi"},
        {L"hr-HR", "hr-HR"},
        {L"hu-HU", "hu"},
        {L"it-IT", "it-IT"},
        {L"ja-JP", "ja"},
        {L"ko-KR", "ko"},
        {L"lt-LT", "lt"},
        {L"lv-LV", "lv"},
        {L"nb-NO", "nb"},
        {L"nl-NL", "nl-NL"},
        {L"pl-PL", "pl"},
        {L"ps-PS", "ps-PS"},
        {L"pt-BR", "pt-BR"},
        {L"pt-PT", "pt-PT"},
        {L"ro-RO", "ro-RO"},
        {L"ru-RU", "ru"},
        {L"sk-SK", "sk"},
        {L"sl-SI", "sl"},
        {L"sr-Latn-CS", "sr-Latn-CS"},
        {L"sv-SE", "sv-SE"},
        {L"th-TH", "th"},
        {L"tr-TR", "tr"},
        {L"uk-UA", "uk"},
        {L"zh-CN", "zh-Hans-CN"},
        {L"zh-HK", "zh-Hant-HK"},
        {L"zh-TW", "zh-Hant-TW"},
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

    std::set<std::string> Applicability::GetLanguages()
    {
        std::set<std::string> result;

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

            std::wistringstream tokenStream(languagesWin7);
            wchar_t delimiter = '\0';
            ULONG processedTags = 0;
            std::wstring muiTag;
            while (std::getline(tokenStream, muiTag, delimiter))
            {
                if (processedTags == numOfLangs) { break; }
                auto it = muiToBcp47.find(muiTag);
                if (it != muiToBcp47.end())
                {
                    result.insert(it->second);
                }
                else
                {   // Is not well known, luckily the tag will be the same (probably not) :)
                    result.insert(utf16_to_utf8(muiTag));
                }
                processedTags++;
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
                result.insert(utf16_to_utf8(hstring.GetRawBuffer(nullptr)));
            }
        }
        return result;
    }
}
