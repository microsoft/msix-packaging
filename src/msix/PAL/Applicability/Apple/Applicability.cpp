//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <CoreFoundation/CoreFoundation.h>
#include <vector>

#include "Applicability.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    MSIX_PLATFORMS Applicability::GetPlatform()
    {
    #ifdef IOS
        return MSIX_PLATFORM_IOS;
    #elif MACOS
        return MSIX_PLATFORM_MACOS;
    #else
        UNEXPECTED;
    #endif
    }

    std::vector<Bcp47Tag> Applicability::GetLanguages()
    {
        std::vector<Bcp47Tag> result;
        CFArrayRef preferredLanguages = CFLocaleCopyPreferredLanguages();
        CFIndex size = CFArrayGetCount(preferredLanguages);
        for(int i = 0; i < size; i++)
        {
            CFStringRef value = static_cast<CFStringRef>(CFArrayGetValueAtIndex(preferredLanguages, i));
            CFIndex valueSize = CFStringGetLength(value) + 1; // +1 for null termination
            char buffer[valueSize];
            ThrowErrorIfNot(Error::Unexpected, CFStringGetCString(value, buffer, valueSize, kCFStringEncodingUTF8),
                "CFStringGetCString conversion failed");
            result.push_back(Bcp47Tag(buffer));
        }
        CFRelease(preferredLanguages);
        return result;
    }
}
