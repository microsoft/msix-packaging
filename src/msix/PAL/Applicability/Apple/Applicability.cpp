//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <CoreFoundation/CoreFoundation.h>
#include <set>

#include "Applicability.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    MSIX_PLATFORM Applicability::GetPlatform()
    {
    #ifdef IOS
        return MSIX_PLATFORM_IOS;
    #elif MACOS
        return MSIX_PLATFORM_MACOS;
    #else
        UNEXPECTED;
    #endif
    }

    std::set<std::string> Applicability::GetLanguages()
    {
        std::set<std::string> result;
        CFArrayRef preferredLanguages = CFLocaleCopyPreferredLanguages();
        CFIndex size = CFArrayGetCount(preferredLanguages);
        for(int i = 0; i < size; i++)
        {
            CFStringRef value = static_cast<CFStringRef>(CFArrayGetValueAtIndex(preferredLanguages, i));
            CFIndex valueSize = CFStringGetLength(value) + 1; // +1 for null termination
            char buffer[valueSize];
            if(CFStringGetCString(value, buffer, valueSize, kCFStringEncodingUTF8))
            {
                result.insert(buffer);
            }
        }
        CFRelease(preferredLanguages);
        return result;
    }
}
