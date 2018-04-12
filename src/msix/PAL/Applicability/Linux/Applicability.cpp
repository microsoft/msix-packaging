//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <unicode/uloc.h>
#include <unicode/utypes.h>
#include <set>

#include "AppxPackaging.hpp"
#include "Applicability.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    MSIX_PLATFORM Applicability::GetPlatform() { return MSIX_PLATFORM_LINUX; }

    std::set<std::string> Applicability::GetLanguages()
    {
        std::set<std::string> result;
        UErrorCode status = U_ZERO_ERROR;
        char bcp47[ULOC_FULLNAME_CAPACITY];
        int bcp47Lenght = uloc_toLanguageTag(uloc_getDefault(), bcp47, ULOC_FULLNAME_CAPACITY, true, &status);
        ThrowErrorIf(Error::Unexpected, U_FAILURE(status) || status == U_STRING_NOT_TERMINATED_WARNING, "Failure getting the system language");
        // TODO: verify if this API returns zh-Hans-CN or zh-CN
        result.insert(bcp47);
        return result;
    }
}
