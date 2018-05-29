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

    MSIX_PLATFORMS Applicability::GetPlatform() { return MSIX_PLATFORM_LINUX; }

    std::vector<Bcp47Tag> Applicability::GetLanguages()
    {
        std::vector<Bcp47Tag> result;
        UErrorCode status = U_ZERO_ERROR;
        char bcp47[ULOC_FULLNAME_CAPACITY] = {};
        int bcp47Length = uloc_toLanguageTag(uloc_getDefault(), bcp47, ULOC_FULLNAME_CAPACITY, true, &status);
        if (U_FAILURE(status) || status == U_STRING_NOT_TERMINATED_WARNING)
        {
            std::ostringstream builder;
            builder << "Failure obtaining system langauge " << u_errorName(status);
            ThrowErrorAndLog(Error::Unexpected, builder.str().c_str());
        }
        result.push_back(Bcp47Tag(bcp47));
        return result;
    }
}
