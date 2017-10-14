#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <initializer_list>
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    namespace Meta {

        // Represents a heterogeneous collection of types that can be operated on as a compile-time vector
        template <typename... Types>
        class TypeList
        {
            std::tuple<Types...> fields;
            static constexpr std::size_t last_index{ std::tuple_size<std::tuple<Types...>>::value };

        public:
            template<std::size_t index = 0, typename FuncT>
            inline typename std::enable_if<index == last_index, void>::type for_each(FuncT)
            { }

            template<std::size_t index = 0, typename FuncT>
            inline typename std::enable_if<index < last_index, void>::type for_each(FuncT f)
            {
                f(Field<index>());
                for_each<index + 1, FuncT>(f);
            }

            template <size_t index>
            inline auto& Field() { return std::get<index>(fields); }
        };

        // Defines set of operations (size, write, read) for a serializable/deserializable type.
        template <class... Types>
        class StructuredObject : public TypeList<Types...>
        {
        public:
            size_t Size()
            {
                size_t result = 0;
                this->for_each([&](auto& item)
                {
                    result += item.Size();
                });
                return result;
            }

            void Write(StreamBase* stream)
            {
                offset = stream->Ftell();
                this->for_each([&](auto& item)
                {
                    item.Write(stream);
                });
            }

            void Read(StreamBase* stream)
            {
                offset = stream->Ftell();
                this->for_each([&](auto& item)
                {
                    item.Read(stream);
                });
            }

            virtual void Validate() { }

        protected:
            std::uint64_t offset = 0;   // For debugging purposes!
        };

        // base type for individual serializable/deserializable fields
        template <class T>
        class FieldBase
        {
        public:
            using Lambda = std::function<void(T& v)>;

            FieldBase()
            {
                validation = [](T&){};    // empty validation by-default.
            }

            virtual void Write(StreamBase* stream)
            {
                StreamBase::Write<T>(stream, &value);
            }

            virtual void Read(StreamBase* stream)
            {
                StreamBase::Read<T>(stream, &value);
                Validate();
            }

            virtual void Validate() { validation(this->value); }
            virtual size_t Size()   { return sizeof(T); }

            T       value;
            Lambda  validation;
        };

        class Field2Bytes : public FieldBase<std::uint16_t> { };
        class Field4Bytes : public FieldBase<std::uint32_t> { };
        class Field8Bytes : public FieldBase<std::uint64_t> { };

        // variable length field.
        class FieldNBytes : public FieldBase<std::vector<std::uint8_t>>
        {
        public:
            virtual size_t Size() override      { return value.size(); }
            virtual void Validate() override    { }

            virtual void Write(StreamBase* stream) override
            {
                stream->Write(Size(), value.data());
            }

            virtual void Read(StreamBase* stream) override
            {
                stream->Read(Size(), value.data());
                Validate();
            }
        };
    }
}
