//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// Wrapper around catch2's REQUIRE
#pragma once

template<typename T>
static bool CompareComPtr(T* const& left, T* const& right)
{
    return (reinterpret_cast<const void* const&>(left) == reinterpret_cast<const void* const&>(right));
}

#define REQUIRE_SUCCEEDED(__hr) { REQUIRE(__hr == S_OK ); }
#define REQUIRE_HR(__exp, __hr) { REQUIRE(__hr == __exp ); }
#define REQUIRE_NOT_NULL(__ptr) { REQUIRE(__ptr != nullptr); }
#define REQUIRE_ARE_SAME(__left, __right) { REQUIRE(CompareComPtr(__left, __right)); }
