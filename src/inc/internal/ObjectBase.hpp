//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <format>
#include <iostream>
#include <vector>
#include <tuple>
#include <type_traits>
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "MsixFeatureSelector.hpp"

namespace MSIX { namespace Meta {
//////////////////////////////////////////////////////////////////////////////////////////////
//                              Basic Validation Policies                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

// there is exactly one value that this field is allowed to be
template <typename T>
static void ExactValueValidation(T value, T spec) {
    std::string message = "Value in field doesn't match expectation; expected " +
        std::to_string(spec) + ", got " + std::to_string(value);
    ThrowErrorIfNot(Error::InvalidParameter, spec == value, message.c_str());
}

// there is exactly one value that this field is not allowed to be
template <typename T>
static void NotValueValidation(T value, T spec) {
    std::string message =
        "Value in field is disallowed match expectation; got disallowed value " +
        std::to_string(value);
    ThrowErrorIf(Error::InvalidParameter, spec == value, message.c_str());
}

// there are exactly two values that this field is allowed to be
template <typename T>
static void OnlyEitherValueValidation(T value, T spec1, T spec2)
{
    std::string message = "Value in field doesn't match expectations; expected either " +
        std::to_string(spec1) + " or " + std::to_string(spec2) + ", got " +
        std::to_string(value);
    ThrowErrorIf(Error::InvalidParameter, 
        spec1 != value && spec2 != value,
        message.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////
//              Base type for individual serializable/deserializable fields                 //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class FieldBase
{
public:
    FieldBase() = default;

    constexpr size_t Size() const { return sizeof(T); }

    void GetBytes(std::vector<std::uint8_t>& bytes) const
    {
        THROW_IF_PACK_NOT_ENABLED
        for (size_t i = 0; i < Size(); ++i)
        {
            bytes.push_back(static_cast<std::uint8_t>(this->value >> (i * 8)));
        }
    }

    FieldBase& operator=(const T& v)
    {
        value = v;
        return *this;
    }

    operator T() const
    {
        return value;
    }

    T& get()
    {
        return value;
    }

    const T& get() const
    {
        return value;
    }

    T* operator &()
    {
        return &value;
    }

protected:
    T value;
};

// Simple 2, 4, and 8 byte fields
using Field2Bytes = FieldBase<std::uint16_t>;
using Field4Bytes = FieldBase<std::uint32_t>;
using Field8Bytes = FieldBase<std::uint64_t>;

// variable length field.
class FieldNBytes : public FieldBase<std::vector<std::uint8_t>>
{
public:
    size_t Size() const { return this->value.size(); }

    void GetBytes(std::vector<std::uint8_t>& result) const
    {
        THROW_IF_PACK_NOT_ENABLED
        result.insert(result.end(), value.begin(), value.end());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//          Base type for individual serializable/deserializable optional fields            //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class OptionalFieldBase
{
public:
    OptionalFieldBase() = default;

    constexpr size_t Size() { return (hasValue ? value.Size() : 0); }

    void GetBytes(std::vector<std::uint8_t>& bytes)
    {
        THROW_IF_PACK_NOT_ENABLED
        if (hasValue)
        {
            value.GetBytes(bytes);
        }
    }

    OptionalFieldBase& operator=(const T& v)
    {
        hasValue = true;
        value = v;
        return *this;
    }

    operator bool() const
    {
        return hasValue;
    }

    operator T() const
    {
        ThrowErrorIfNot(Error::InvalidState, hasValue, "Cannot retrieve value if none is set");
        return value;
    }

    const T& get() const
    {
        ThrowErrorIfNot(Error::InvalidState, hasValue, "Cannot retrieve value if none is set");
        return value.get();
    }

    T* operator &()
    {
        hasValue = true;
        return &value;
    }

protected:
    FieldBase<T> value;
    bool hasValue = false;
};

// Simple 2, 4, and 8 byte fields
using OptionalField2Bytes = OptionalFieldBase<std::uint16_t>;
using OptionalField4Bytes = OptionalFieldBase<std::uint32_t>;
using OptionalField8Bytes = OptionalFieldBase<std::uint64_t>;

//////////////////////////////////////////////////////////////////////////////////////////////
//      Heterogeneous collection of types that are operated on as a compile-time vector     //
//////////////////////////////////////////////////////////////////////////////////////////////
template <typename... Types>
class TypeList
{
    static constexpr std::size_t last_index { sizeof...(Types) };
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

    template <size_t index>
    const auto& Field() const noexcept { return std::get<index>(fields); }
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
        {
            result += field.Size();
        }, result);
        return result;
    }

    std::vector<std::uint8_t> GetBytes()
    {
        THROW_IF_PACK_NOT_ENABLED
        std::vector<std::uint8_t> bytes;
        this->for_each([](auto& field, std::size_t index, std::vector<std::uint8_t>& bytes)
        {
            field.GetBytes(bytes);
        }, bytes);
        return bytes;
    }

    void WriteTo(const ComPtr<IStream>& stream)
    {
        THROW_IF_PACK_NOT_ENABLED
        auto bytes = GetBytes();
        ULONG bytesWritten = 0;
        ThrowHrIfFailed(stream->Write(bytes.data(), static_cast<ULONG>(bytes.size()), &bytesWritten));
    }
};

} /* namespace Meta */ } /* namespace MSIX */
