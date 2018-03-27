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
template <typename Derived>
class NoValidation
{
public:
    static inline void Validate(std::size_t, Derived*) { }
};

template <typename Derived>
class InvalidFieldValidation // If the field has size>0, then this field should NOT be read.
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIf(Error::Unexpected, self->Size() != 0, "Unexpected data encountered.");
    }
};
    
template <typename Derived, std::size_t spec>
class ExactValueValidation // there is exactly one value that this field is allowed to be
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIfNot(Error::InvalidParameter, spec == self->value, "Incorrect value specified at field.");
    }
};

template <typename Derived, std::size_t spec>
class NotValueValidation // there is exactly one value that this field is not allowed to be
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIf(Error::InvalidParameter, spec == self->value, "Incorrect value specified at field.");
    }
};

template <typename Derived, std::size_t spec1, std::size_t spec2>
class OnlyEitherValueValidation // there are exactly two values that this field is allowed to be
{
public:
    static void Validate(std::size_t, Derived* self)
    {
        ThrowErrorIf(Error::InvalidParameter, 
            spec1 != self->value && spec2 != self->value,
            "Incorrect value specified at field.");
    }
};
    
//////////////////////////////////////////////////////////////////////////////////////////////
//                           Advanced Validation Policies                                   //
//////////////////////////////////////////////////////////////////////////////////////////////

// An injectable validator type supports parent-injectable validation.  This is used in
// concert with the InjectedValidation type, which is specified as a validator of either
// FieldBase or FieldNBytes
class InjectableValidator
{
public:
    virtual void ValidateField(size_t field)
    {
        // If you're here and you didn't override, you either have a bug in your 
        // type list definition or your derived type didn't wire-up the specified
        // field's parent to point at your type's instance.
        UNEXPECTED;
    }
};

template <typename Derived>
class InjectedValidation  // derived type will rely on in situ parent-scope validation 
{
public:
    static void Validate(std::size_t field, Derived* self)  { self->parent->ValidateField(field); }

    InjectableValidator* parent = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//              Base type for individual serializable/deserializable fields                 //
//////////////////////////////////////////////////////////////////////////////////////////////
template <typename Derived, class T, class Validation=NoValidation<Derived> >
class FieldBase : public Validation
{
public:
    FieldBase() = default;
 
    void Write(std::size_t index, const ComPtr<IStream>& stream)
    {
        StreamBase::Write<T>(stream, &value);
    }
    
    void Read(std::size_t index, const ComPtr<IStream>& stream)
    {
        StreamBase::Read<T>(stream, &value);
        this->Validate(index, static_cast<Derived*>(this));
    }
    
    size_t Size() { return sizeof(T); }
    
    T value;
};

// Simple 2, 4, and 8 byte fields with no validation
class Field2Bytes : public FieldBase<Field2Bytes, std::uint16_t> { };
class Field4Bytes : public FieldBase<Field4Bytes, std::uint32_t> { };
class Field8Bytes : public FieldBase<Field8Bytes, std::uint64_t> { };

// variable length field.
template <typename Derived, class Validation=NoValidation<Derived> >
class VarLenField : public FieldBase<Derived, std::vector<std::uint8_t>, Validation>
{
public:
    size_t Size() { return this->value.size(); }
    
    void Write(std::size_t index, const ComPtr<IStream>& stream)
    {
        if (this->value.size() != 0)
        {   ThrowHrIfFailed(stream->Write(
                reinterpret_cast<void*>(this->value.data()),
                static_cast<ULONG>(this->value.size()),
                nullptr
            )); 
        }       
    }
    
    void Read(std::size_t index, const ComPtr<IStream>& stream)
    {
        if (this->value.size() != 0)
        {   ThrowHrIfFailed(stream->Read(
                reinterpret_cast<void*>(this->value.data()),
                static_cast<ULONG>(this->value.size()),
                nullptr                            
            ));
        }
        this->Validate(index, static_cast<Derived*>(this));
    }
};

class FieldNBytes : public VarLenField<FieldNBytes> {};

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
template <typename Derived, class... Types>
class StructuredObject : public InjectableValidator, public TypeList<Types...>
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
    
    void Write(const ComPtr<IStream>& s)
    {
        this->for_each([](auto& field, std::size_t index, const ComPtr<IStream>& stream)
        {   field.Write(index, stream);
        }, s);
    }
    
    void Read(const ComPtr<IStream>& s)
    {
        this->for_each([](auto& field, std::size_t index, const ComPtr<IStream>& stream)
        {  field.Read(index, stream);
        }, s);
        static_cast<Derived*>(this)->Validate();
    }

    void Validate() {} // default structured object validation is no validation.

    template<size_t index>
    void ConfigureField() { std::get<index>(this->fields).parent = static_cast<InjectableValidator*>(this); }
};

} /* namespace Meta */ } /* namespace MSIX */