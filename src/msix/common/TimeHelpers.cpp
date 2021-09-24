//
//  Copyright (C) 2021 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "TimeHelpers.hpp"

#include <chrono>

namespace MSIX {

    MsDosDateAndTime::MsDosDateAndTime()
    {
        // Convert system time to local time so the zip file item can have the same time of the zip archive when
        // it is shown in zip utility tools.
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        const auto localTime = std::localtime(&now);
        Initialize(localTime);
    }

    MsDosDateAndTime::MsDosDateAndTime(const std::tm* time)
    {
        Initialize(time);
    }

    void MsDosDateAndTime::Initialize(const std::tm* time)
    {
            // tm struct tm_year is the years since 1900, so we add them back to validate.
        auto year = time->tm_year + 1900;

        // The MS-DOS date format can represent only dates between 1/1/1980 and 12/31/2107;
        if (year < 1980 || year > 2107)
        {
            // We default the time to 12/31/2107, 23:59:58.
            // Note that MS-DOS time format cannot represent odd seconds. So the nearest time
            // to 23:59:59 that can be represented in MS-DOS time is 23:59:58.
            m_dosTime = 0xBF7D;
            m_dosDate = 0xFF9F;
        }
        else
        {
            // Windows implementation uses TIME_FIELDS struct to calculate the MS-DOS time.
            // There are some differences between it and tm.
            //  | Unit  | TIME_FILES | tm
            //  |-------|------------|----
            //  | year  | since 1601 | since 1900
            //  | month | 1-12       | 0-11
            //  | day   | 1-31       | 1-31
            //  | hour  | 0-23       | 0-23
            //  | min   | 0-59       | 0-59
            //  | sec   | 0-59       | 0-60 (Range allows for a positive leap second.)

            std::uint16_t toConvertYear = static_cast<std::uint16_t>(year - 1980);
            std::uint16_t toConvertMonth = static_cast<std::uint16_t>(time->tm_mon + 1);
            std::uint16_t toConvertDay = static_cast<std::uint16_t>(time->tm_mday);
            std::uint16_t toConvertHour = static_cast<std::uint16_t>(time->tm_hour);
            std::uint16_t toConvertMinute = static_cast<std::uint16_t>(time->tm_min);
            std::uint16_t toConvertSecond = static_cast<std::uint16_t>(time->tm_sec);

            // Ignore leap second.
            if (toConvertSecond == 60)
            {
                toConvertSecond--;
            }

            // MS-DOS date is a packed value with the following format.
            //  | Bits | Description
            //  |------|------------|----
            //  | 0–4  | Day of the month (1–31)
            //  | 5–8  | Month (1 = January, 2 = February, etc.)
            //  | 9-15 | Year offset from 1980 (add 1980 to get actual year)
            m_dosDate = (toConvertYear << 9) |
                        (toConvertMonth << 5) |
                        toConvertDay;

            // MS-DOS time is a packed value with the following format.
            //  | Bits  | Description
            //  |-------|------------|----
            //  | 0–4   | Second divided by 2
            //  | 5–10  | Minute (0–59)
            //  | 11-15 | Hour (0–23 on a 24-hour clock)
            m_dosTime = (toConvertHour << 11) |
                        (toConvertMinute << 5) |
                        toConvertSecond / 2;
        }
    }

} // namespace MSIX
