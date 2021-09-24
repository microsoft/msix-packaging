//
//  Copyright (C) 2021 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <cstdint>
#include <ctime>

namespace MSIX {

    // The date and time stored in zip headers is in standard MS-DOS format.
    // This class converts from local system time to MS-DOS date/time format.
    struct MsDosDateAndTime
    {
        MsDosDateAndTime();
        MsDosDateAndTime(const std::tm* time);

        std::uint16_t GetDosTime() { return m_dosTime; }
        std::uint16_t GetDosDate() { return m_dosDate; }

    private:
        std::uint16_t m_dosTime;
        std::uint16_t m_dosDate;

        void Initialize(const std::tm* time);
    };
}
