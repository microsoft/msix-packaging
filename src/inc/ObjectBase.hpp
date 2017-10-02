#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <initializer_list>
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {

    namespace Meta {

        // A uniary type with Read, Write, Validate, and Size
        class Object
        {
        public:
            Object(void* value) : v(value) {}
            virtual ~Object() { }

            virtual void Write() = 0;
            virtual void Read() = 0;
            virtual void Validate() = 0;
            virtual size_t Size() = 0;

            template <class T>
            static T* GetValue(Object* o)
            {
                return reinterpret_cast<T*>(o->v);
            }

            template <class T>
            static void SetValue(Object* o, T& value)
            {
                *reinterpret_cast<T*>(o->v) = value;
            }

        protected:
            void* value() { return v; }
            void* v = nullptr;
        };

        // Aggregates a collection of objects
        //      validation is handled incrementally via Read
        //      size is summation of size of all fields.
        class StructuredObject : public Object
        {
        public:
            StructuredObject(std::initializer_list<std::shared_ptr<Object>> list) : fields(list), Object(&fields) { }

            virtual void Write() override
            {
                std::for_each(fields.begin(), fields.end(), [&](auto field) { field->Write(); });
            }

            virtual void Read() override
            {
                std::for_each(fields.begin(), fields.end(), [&](auto field)
                {
                    field->Read();
                    field->Validate();
                });
            }

            virtual void Validate() override {}

            virtual size_t Size() override
            {
                size_t result = 0;
                std::for_each(fields.begin(), fields.end(), [&](auto field) { result += field->Size(); });
                return result;
            }

            Object* Field(size_t index) { return fields[index].get(); }

        protected:
            std::vector<std::shared_ptr<Object>> fields;
        };

        // base type for serializable fields
        template <class T>
        class FieldBase : public Object
        {
        public:
            using Lambda = std::function<void(T& v)>;

            FieldBase(StreamBase* stream, Lambda validator) : stream(stream), validate(validator), Object(&value) {}

            virtual T&   GetValue()     { return value; }
            virtual void SetValue(T& v) { value = v; }

            virtual void Write() override
            {
                offset = stream->Ftell();
                stream->Write(sizeof(T), reinterpret_cast<std::uint8_t*>(const_cast<T*>(&value)));
            }

            virtual void Read() override
            {
                offset = stream->Ftell();
                stream->Read(sizeof(T), reinterpret_cast<std::uint8_t*>(const_cast<T*>(&value)));
                Validate();
            }

            void Validate() override { validate(GetValue()); }

            virtual size_t Size() override { return sizeof(T); }

        protected:
            std::uint64_t offset = 0;
            T value;
            StreamBase* stream;
            Lambda validate;
        };

        // 2 byte field
        class Field2Bytes : public FieldBase<std::uint16_t>
        {
        public:
            Field2Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint16_t>(stream, validator) {}
        };

        // 4 byte field
        class Field4Bytes : public FieldBase<std::uint32_t>
        {
        public:
            Field4Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint32_t>(stream, validator) {}
        };

        // 8 byte field
        class Field8Bytes : public FieldBase<std::uint64_t>
        {
        public:
            Field8Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint64_t>(stream, validator) {}
        };

        // variable length field.
        class FieldNBytes : public Object
        {
        public:
            using Lambda = std::function<void(std::vector<std::uint8_t>& v)>;
            FieldNBytes(StreamBase* stream, Lambda validator) : stream(stream), validate(validator), Object(&value) {}

            size_t Size() override { return value.size(); }

            virtual void Write() override
            {
                stream->Write(Size(), value.data());
            }

            virtual void Read() override
            {
                stream->Read(Size(), value.data());
                Validate();
            }

            void Validate() override { validate(value); }

        protected:
            std::vector<std::uint8_t> value;
            StreamBase* stream;
            Lambda validate;
        };
    }
}