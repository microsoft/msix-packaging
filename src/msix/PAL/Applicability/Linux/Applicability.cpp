//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <unicode/uloc.h>
#include <unicode/utypes.h>
#include <vector>
#include <iterator>
#include <algorithm>

#include "Applicability.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    struct Bcp47Entry
    {
        const char* icu;
        const char* bcp47;

        Bcp47Entry(const char* i, const char* b) : icu(i), bcp47(b) {}

        inline bool operator==(const char* otherIcui) const {
            return 0 == strcmp(icu, otherIcui);
        }
    };

    // We've seen cases were uloc_toLanguageTag returns zh-CN. Add here any inconsistencies.
    static const Bcp47Entry bcp47List[] = {
        Bcp47Entry(u8"zh-CN", u8"zh-Hans-CN"),
        Bcp47Entry(u8"zh-HK", u8"zh-Hant-HK"),
        Bcp47Entry(u8"zh-TW", u8"zh-Hant-TW"),
    };

    MSIX_PLATFORMS Applicability::GetPlatform() { return MSIX_PLATFORM_LINUX; }

    std::vector<std::string> Applicability::GetLanguages()
    {
        std::vector<std::string> result;
        UErrorCode status = U_ZERO_ERROR;
        char bcp47[ULOC_FULLNAME_CAPACITY] = {};
        int bcp47Length = uloc_toLanguageTag(uloc_getDefault(), bcp47, ULOC_FULLNAME_CAPACITY, true, &status);
        if (U_FAILURE(status) || status == U_STRING_NOT_TERMINATED_WARNING)
        {
            std::ostringstream builder;
            builder << "Failure obtaining system langauge " << u_errorName(status);
            ThrowErrorAndLog(Error::Unexpected, builder.str().c_str());
        }
        const auto& tag = std::find(std::begin(bcp47List), std::end(bcp47List), bcp47);
        if (tag == std::end(bcp47List))
        {
            result.push_back(bcp47);
        }
        else
        {
            result.push_back((*tag).bcp47);
        }
        return result;
    }
}
