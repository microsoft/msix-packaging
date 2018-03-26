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
template <class Derived>
class NoValidation
{
public:
    static inline void Validate(std::size_t, Derived*) { }
};

template <class Derived>
class InvalidFieldValidation // If the field has size>0, then this field should NOT be read.
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIf(Error::Unexpected, self->Size() != 0, "Unexpected data encountered.");
    }
};
    
template <class Derived, std::size_t spec>
class ExactValueValidation // there is exactly one value that this field is allowed to be
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIfNot(Error::InvalidParameter, spec == self->value, "Incorrect value specified at field.");
        }
    }
};

template <class Derived, std::size_t spec>
class NotValueValidation // there is exactly one value that this field is not allowed to be
{
public:
    static void Validate(std::size_t, Derived* self) {
        ThrowErrorIf(Error::InvalidParameter, spec == self->value, "Incorrect value specified at field.");
        }
    }
};

template <class Derived, std::size_t spec1, std::size_t spec2>
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
    
template <class Derived, std::size_t spec1, std::size_t spec2>
class NeitherValueValidation // 2's compliment to previous
{
public:
    static void Validate(std::size_t, Derived* self)
    {
        ThrowErrorIf(Error::InvalidParameter,
            spec1 == self->value || spec2 == self->value,
            "Incorrect value specified at field.");
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                           Advanced Validation Policies                                   //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class Derived>
class VirtualValidation // let the derived type specify what-ever elaborate story they want
{
public:
    static inline void Validate(std::size_t, Derived* self)  { self->Validate(); }
};

// An injectable validator type supports parent-injectable validation.  This is used in
// concert with the InjectedValidation type, which is specified as a validator of either
// FieldBase or FieldNBytes
class InjectableValidator
{
public:
    virtual void ValidateField(size_t field) = 0;

    template<size_t index>
    inline void  ConfigureField() noexcept { this->Field<index>().parent = static_cast<InjectableValidator*>(this); }
};

template <class Derived>
class InjectedValidation  // derived type will rely on in situ parent-scope validation 
{
public:
    static inline void Validate(std::size_t field, Derived* self)  { parent->ValidateField(field); }

    InjectableValidator* parent = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//              Base type for individual serializable/deserializable fields                 //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class Derived, class T, class Validation=NoValidation<Derived> >
class FieldBase
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
        Validation::Validate(static_cast<Derived*>(index, this));
    }
    
    inline size_t Size() const  { return sizeof(T); }
    
    T value;
};

// Simple 2, 4, and 8 byte fields with no validation
class Field2Bytes : public FieldBase<Field2Bytes, std::uint16_t> { };
class Field4Bytes : public FieldBase<Field4Bytes, std::uint32_t> { };
class Field8Bytes : public FieldBase<Field8Bytes, std::uint64_t> { };

// variable length field.
template <class Derived, class Validation=NoValidation<Derived> >
class FieldNBytes
{
public:
    inline size_t Size() const { return value.size(); }
    
    void Write(std::size_t index, const ComPtr<IStream>& stream)
    {
        if (value.size() != 0)
        {   ThrowHrIfFailed(stream->Write(
                reinterpret_cast<void*>(value.data()),
                static_cast<ULONG>(value.size()),
                nullptr
            )); 
        }       
    }
    
    void Read(std::size_t index, const ComPtr<IStream>& stream)
    {
        if (value.size() != 0)
        {   ThrowHrIfFailed(stream->Read(
                reinterpret_cast<void*>(value.data()),
                static_cast<ULONG>(value.size()),
                nullptr                            
            ));
        }        
        Validation::Validate(static_cast<Derived*>(index, this));
    }
    
    std::vector<std::uint8_t> value;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//      Heterogeneous collection of types that are operated on as a compile-time vector     //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class Derived, typename... Types>
class TypeList : public std::tuple<Types...>
{
    static constexpr std::size_t last_index { std::tuple_size<std::tuple<Types...>>::value };
public:
    template<std::size_t index = 0, typename FuncT, class... Args>
    inline typename std::enable_if<index == last_index, void>::type for_each(FuncT, Args&&... args) { }

    template<std::size_t index = 0, typename FuncT, class... Args>
    inline typename std::enable_if<index < last_index, void>::type for_each(FuncT f, Args&&... args)
    {
        f(Field<index>(), index, std::forward<Args>(args)...);
        for_each<index + 1, FuncT>(f, std::forward<Args>(args)...);
    }

    template <size_t index>
    inline auto& Field() { return std::get<index>(*static_cast<Derived*>(this)); }
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                              Aggregated set of types                                     //
//////////////////////////////////////////////////////////////////////////////////////////////
template <class Derived, class... Types>
class StructuredObject : public TypeList<Derived, Types...>, Validation=VirtualValidation<Derived>, InjectableValidator
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
        Validation::Validate(static_cast<Derived*>(this));
    }

    void Validate() override {} // default sturcutred object validation is no validation.
    void ValidateField(size_t field) override
    {   // If you're here and you didn't override, you either have a bug in your 
        // type list definition or your derived type didn't wire-up the specified
        // field's parent to point at your type's instance.
        UNEXPECTED;
    }
};

} /* namespace Meta */ } /* namespace MSIX */