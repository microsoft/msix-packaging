
#include <vector>
#include <functional>
#include <initializer_list>
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {

    class ObjectBase
    {
    public:
        ObjectBase() {}
        virtual ~ObjectBase() {}

        virtual void Write() = 0;
        virtual void Read() = 0;
        virtual void Validate() {}

        template <class T> virtual T& Value()              { throw NotSupportedException(); }
        template <class T> virtual void SetValue(T& value) { throw NotSupportedException(); }
    };

    class StructuredObject : public ObjectBase
    {
    public:
        StructuredObject(std::initializer_list<ObjectBase> fields) : list(fields) { }

        virtual void Write()
        {
            for (auto field : list)
            {
                field.Validate();
                field.Write();
            }
        }

        virtual void Read()
        {
            for (auto field : list)
            {
                field.Read();
                field.Validate();
            }
        }

        ObjectBase& Field(size_t index) { return list[index]; }

        std::vector<ObjectBase>& Value() { return list; }

    protected:
        std::vector<ObjectBase> list;
    };

    namespace Meta {
        template <class T>
        class FieldBase : public ObjectBase
        {
        public:
            using Lambda = std::function<void(T& v)>;

            FieldBase(StreamBase& stream, Lambda validator) : stream(stream), validate(validator) {}

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

            T& Value() { return value; }
            void SetValue(T& v) { value = v; }

        protected:
            T value;
            StreamBase& stream;
            Lambda validate;
        };

        class Field2Bytes : public FieldBase<std::uint16_t>
        {
        public:
            Field2Bytes(StreamBase& stream, Lambda&& validator) : FieldBase<std::uint16_t>(stream, validator) {}
        };

        class Field4Bytes : public FieldBase<std::uint32_t>
        {
        public:
            Field4Bytes(StreamBase& stream, Lambda&& validator) : FieldBase<std::uint32_t>(stream, validator) {}
        };

        class Field8Bytes : public FieldBase<std::uint64_t>
        {
        public:
            Field8Bytes(StreamBase& stream, Lambda&& validator) : FieldBase<std::uint64_t>(stream, validator) {}
        };
    }
}