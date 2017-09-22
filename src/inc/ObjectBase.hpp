#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <initializer_list>
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {

    class ObjectBase
    {
    public:
        ObjectBase(void* value) : v(value) {}
        virtual ~ObjectBase() { }

        virtual void Write() = 0;
        virtual void Read() = 0;
        virtual void Validate() = 0;
        virtual size_t Size() = 0;

        template <class T> static T&   GetValue(ObjectBase& o)           { return static_cast<T&>(o.v);      }
        template <class T> static void SetValue(ObjectBase& o, T& value) { o.v = static_cast<void*>(&value); }

    protected:
        void* value() { return v; }
        void setValue(void* value) { v = value; }

        void* v = nullptr;
    };

    class StructuredObject : public ObjectBase
    {
    public:
        StructuredObject(std::initializer_list<std::shared_ptr<ObjectBase>> fields) : fields(fields), ObjectBase(&fields) { }

        virtual void Write()
        {
            for (auto field : fields)
            {
                field->Validate();
                field->Write();
            }
        }

        virtual void Read()
        {
            for (auto field : fields)
            {
                field->Read();
                field->Validate();
            }
        }

        virtual void Validate() {}

        virtual size_t Size()
        {
            size_t result = 0;
            for (auto field : fields)
            {
                result += field->Size();
            }
        }

        ObjectBase* Field(size_t index) { return fields[index].get(); }

    protected:
        std::vector<std::shared_ptr<ObjectBase>> fields;
    };

    namespace Meta {
        template <class T>
        class FieldBase : public ObjectBase
        {
        public:
            using Lambda = std::function<void(T& v)>;

            FieldBase(StreamBase* stream, Lambda validator) : stream(stream), validate(validator) {}

            virtual T& Value()          { return value; }
            virtual void SetValue(T& v) { value = v; }

            virtual void Write()
            {
                Validate();
                stream.Write(sizeof(T), static_cast<std::uint8_t>(const_cast<T>(&value)));
            }

            virtual void Read()
            {
                stream.Read(sizeof(T), static_cast<std::uint8_t>(const_cast<T>(&value)));
                Validate();
            }

            void Validate() { validate(Value()); }

            virtual size_t Size() { return sizeof(T); }

        protected:
            T value;
            StreamBase* stream;
            Lambda validate;
        };

        class Field2Bytes : public FieldBase<std::uint16_t>
        {
        public:
            Field2Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint16_t>(stream, validator) {}
        };

        class Field4Bytes : public FieldBase<std::uint32_t>
        {
        public:
            Field4Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint32_t>(stream, validator) {}
        };

        class Field8Bytes : public FieldBase<std::uint64_t>
        {
        public:
            Field8Bytes(StreamBase* stream, Lambda&& validator) : FieldBase<std::uint64_t>(stream, validator) {}
        };

        class FieldNBytes : public FieldBase<std::vector<std::uint8_t>>
        {
        public:
            using Lambda = std::function<void(std::vector<std::uint8_t>& v)>;
            FieldNBytes(StreamBase* stream, Lambda validator) : FieldBase(stream, validator) {}

            size_t Size() { return Value().size(); }

            virtual void Write()
            {
                Validate();
                stream->Write(Size(), static_cast<std::uint8_t>(const_cast<T>(Value().data())));
            }

            virtual void Read()
            {
                stream->Read(Size(), static_cast<std::uint8_t>(const_cast<T>(Value().data())));
                Validate();
            }
        };
    }
}