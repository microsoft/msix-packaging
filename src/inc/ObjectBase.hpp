//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>
#include <tuple>
#include <type_traits>
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace MSIX { namespace Meta {
//////////////////////////////////////////////////////////////////////////////////////////////
//                              Basic Validation Policies                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

// there is exactly one value that this field is allowed to be
template <typename T>
static void ExactValueValidation(T value, T spec) {
    ThrowErrorIfNot(Error::InvalidParameter, spec == value, "Incorrect value specified at field.");
}

// there is exactly one value that this field is not allowed to be
template <typename T>
static void NotValueValidation(T value, T spec) {
    ThrowErrorIf(Error::InvalidParameter, spec == value, "Incorrect value specified at field.");
}

// there are exactly two values that this field is allowed to be
template <typename T>
static void OnlyEitherValueValidation(T value, T spec1, T spec2)
{
    ThrowErrorIf(Error::InvalidParameter, 
        spec1 != value && spec2 != value,
        "Incorrect value specified at field.");
}

//////////////////////////////////////////////////////////////////////////////////////////////
//              Base type for individual serializable/deserializable fields                 //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class FieldBase
{
public:
    FieldBase() = default;

    size_t Size() { return sizeof(T); }
    
    T value;
};

// Simple 2, 4, and 8 byte fields
class Field2Bytes final : public FieldBase<std::uint16_t> { };
class Field4Bytes final : public FieldBase<std::uint32_t> { };
class Field8Bytes final : public FieldBase<std::uint64_t> { };

// variable length field.
class FieldNBytes : public FieldBase<std::vector<std::uint8_t>>
{
public:
    size_t Size() { return this->value.size(); }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//      Heterogeneous collection of types that are operated on as a compile-time vector     //
//////////////////////////////////////////////////////////////////////////////////////////////
template <typename... Types>
class TypeList
{
    static constexpr std::size_t last_index { std::tuple_size<std::tuple<Types...>>::value };
public:
    std::tuple<Types...> fields;    
    template<std::size_t index = 0, typename FuncT, class... Args>
    inline typename std::enable_if<index == last_index, void>::type for_each(FuncT, Args&&... args) { }

    template<std::size_t index = 0, typename FuncT, class... Args>
    inline typename std::enable_if<index < last_index, void>::type for_each(FuncT f, Args&&... args)
    {
        f(Field<index>(), index, std::forward<Args>(args)...);
        for_each<index + 1, FuncT>(f, std::forward<Args>(args)...);
    }

    template <size_t index>
    auto& Field() noexcept { return std::get<index>(fields); }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                              Aggregated set of types                                     //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class... Types>
class StructuredObject : public TypeList<Types...>
{
public:
    size_t Size()
    {
        size_t result = 0;
        this->for_each([](auto& field, std::size_t index, size_t& result)
        {   result += field.Size();
        }, result);
        return result;
    }
};

} /* namespace Meta */ } /* namespace MSIX */