//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <unicode/uloc.h>
#include <unicode/utypes.h>
#include <vector>
#include <iterator> 

#include "Applicability.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    struct Bcp47Entry
    {
        const char* icu;
        const char* bcp47;

        Bcp47Entry(const char* i, const char* b) : icu(i), bcp47(b) {}

        inline bool operator==(const Bcp47Entry& rhs) const {
            return 0 == strcmp(icu, rhs.icu);
        }
    };

    // We've seen cases were uloc_toLanguageTag returns zh-CN. Add here any inconsistencies.
    static const Bcp47Entry bcp47List[] = {
        Bcp47Entry("zh-CN", "zh-Hans-CN"),
        Bcp47Entry("zh-HK", "zh-Hant-HK"),
        Bcp47Entry("zh-TW", "zh-Hant-TW"),
    };

    MSIX_PLATFORM Applicability::GetPlatform() { return MSIX_PLATFORM_LINUX; }

    std::vector<std::string> Applicability::GetLanguages()
    {
        std::vector<std::string> result;
        UErrorCode status = U_ZERO_ERROR;
        char bcp47[ULOC_FULLNAME_CAPACITY] = {};
        int bcp47Length = uloc_toLanguageTag(uloc_getDefault(), bcp47, ULOC_FULLNAME_CAPACITY, true, &status);
        std::ostringstream builder;
        builder << "Failure obtaining system langauge " << u_errorName(status);
        ThrowErrorIf(Error::Unexpected, U_FAILURE(status) || status == U_STRING_NOT_TERMINATED_WARNING, builder.str().c_str());
        const auto& tag = std::find(std::begin(bcp47List), std::end(bcp47List), Bcp47Entry(bcp47, nullptr));
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
