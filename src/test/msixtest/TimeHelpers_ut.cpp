//
//  Copyright (C) 2021 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "catch.hpp"
#include "TimeHelpers.hpp"

#include <iostream>

using namespace MSIX;

TEST_CASE("TimeHelper_UT", "[unittests]")
{
    std::tm time {};
    time.tm_year = 1991-1900; // 1991
    time.tm_mon = 11 - 1; // November
    time.tm_mday = 3;
    time.tm_hour = 10;
    time.tm_min = 27;
    time.tm_sec = 12;

    auto msDosDateAndTime = new MsDosDateAndTime(&time);
    CHECK(0x1763 == msDosDateAndTime->GetDosDate());
    CHECK(0x5366 == msDosDateAndTime->GetDosTime());
}

TEST_CASE("TimeHelper_boundaries_UT", "[unittests]")
{
    std::tm future {};
    future.tm_year = 2117-1900; // 2117
    future.tm_mon = 11 - 1; // November
    future.tm_mday = 3;
    future.tm_hour = 10;
    future.tm_min = 27;
    future.tm_sec = 12;

    auto msDosDateAndTimeFuture = new MsDosDateAndTime(&future);
    CHECK(0xff9f == msDosDateAndTimeFuture->GetDosDate());
    CHECK(0xbf7d == msDosDateAndTimeFuture->GetDosTime());

    std::tm past {};
    past.tm_year = 1254-1900; // 1254
    past.tm_mon = 11 - 1; // November
    past.tm_mday = 3;
    past.tm_hour = 10;
    past.tm_min = 27;
    past.tm_sec = 12;

    auto msDosDateAndTimePast = new MsDosDateAndTime(&past);
    CHECK(0xff9f == msDosDateAndTimePast->GetDosDate());
    CHECK(0xbf7d == msDosDateAndTimePast->GetDosTime());
}
