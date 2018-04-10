//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include <windows.h>
#include <VersionHelpers.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <windows.system.userprofile.h>

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "Applicability.hpp"
#include "ComHelper.hpp"
#include "UnicodeConversion.hpp"

#include <set>
#include <iostream>
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::System::UserProfile;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Foundation;


namespace MSIX {

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
            // TODO: split languagesWin7 by /0 and add the table with MUI to BCP47 conversion
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
