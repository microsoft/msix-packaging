#pragma once

#include <assert.h>

#ifndef APPMODEL_WIDESTRING_H_
#define APPMODEL_WIDESTRING_H_
#define COMMON_STRING_EMPTY { 0, NULL }

#include <IntSafe.h>

namespace MsixCoreLib
{
    struct COMMON_STRING
    {
        ULONG length;
        __field_nullterminated __field_ecount(length + 1) WCHAR* chars;
    };

    struct CONST_STRING
    {
        ULONG length;
        __field_nullterminated __field_ecount(length + 1) const WCHAR* chars;
    };

    static const ULONG MaxValue = 0xFFFFFFFF;

    static HRESULT Multiply(
        _In_ ULONG d1,
        _In_ ULONG d2,
        _Out_ ULONG *result)
    {
        (*result) = d1 * d2;
        if ((d2 == 0 || d1 <= MaxValue / d2))
        {
            return NOERROR;
        }
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }

    static HRESULT AddNew(
        _In_ ULONG d1,
        _In_ ULONG d2,
        __out_range(>= , d1) __out_range(>= , d2) ULONG *result)
    {
        (*result) = d1 + d2;
        if (d1 <= MaxValue - d2)
        {
            return NOERROR;
        }
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }

    class StringBuffer
    {
    private:
        COMMON_STRING value = {}; // value.length does not include terminating NULL
        ULONG bufferLength = 0; // bufferLength includes space for the terminating NULL
        static const ULONG minCapacity = 32;
        static const ULONG initialCapacity = 8;

    public:

        static const ULONG MaxLength = 0x7FFFFFFE / sizeof(WCHAR);
        StringBuffer() = default;

        StringBuffer(const StringBuffer&) = delete;
        void operator=(const StringBuffer&) = delete;

        StringBuffer(StringBuffer&& other) :
            value(other.value),
            bufferLength(other.bufferLength)
        {
            other.value = {};
            other.bufferLength = 0;
        }

        StringBuffer& operator=(StringBuffer&& other)
        {
            if (this != &other)
            {
                Reset();
                value = other.value;
                bufferLength = other.bufferLength;

                other.value = {};
                other.bufferLength = 0;
            }

            return *this;
        }

        // this method assumes ownership of the passed in string. It
        // must be allocated with new and will be freed by the destructor
        HRESULT InitializeFromString(
            __in_z_opt WCHAR *string);

        ~StringBuffer()
        {
            // If the user of StringBuffer isn't using the GlobalHeap, there is no
            // guarantee that deleting a null pointer won't result in a heap corruption
            // break. The RTL heap, for example, issues a heap corruption failure
            // in this case. As a result, ensure that NULL isn't passed to heap free routines.
            if (this->value.chars != NULL)
            {
                delete[] this->value.chars;
            }
        }

        inline const COMMON_STRING* GetString() const
        {
            return &value;
        }
        inline COMMON_STRING* GetString()
        {
            return &value;
        }

        inline const COMMON_STRING& GetStringRef() const
        {
            return value;
        }
        inline COMMON_STRING& GetStringRef()
        {
            return value;
        }

        inline const WCHAR* GetChars() const
        {
            return value.chars;
        }
        WCHAR* GetChars()
        {
            return value.chars;
        }

        // returns # of valid characters in the buffer (excludes zero terminator)
        ULONG GetLength() const
        {
            return value.length;
        }

        // returns the # of characters that can fit in the current capacity of the buffer
        ULONG GetCapacity() const
        {
            return (bufferLength == 0) ? 0 : bufferLength - 1;
        }

        WCHAR* GetRange(
            _In_ ULONG offset,
            _In_ ULONG count) const
        {
            UNREFERENCED_PARAMETER(count);
            assert(offset < this->value.length &&
                count <= this->value.length && offset <= this->value.length - count);
            return &value.chars[offset];
        }

        void Clear();

        // returns string to original empty state
        void Reset()
        {
            delete[] this->value.chars;
            this->value.chars = NULL;
            this->value.length = 0;
            this->bufferLength = 0;
        }

        // Updates the length of the valid characters in the buffer, increases the capacity if needed.
        HRESULT SetLength(
            _In_ ULONG length);

        // Adjusts (increase or decrease) the capacity to handle "length" number of characters
        HRESULT SetCapacity(
            _In_ ULONG capacity);

        // this method makes a copy of the passed in string. Ownership
        // of the passed in string remains with the caller. Length does not include terminating NULL
        HRESULT SetValue(
            _In_reads_(length) const WCHAR* chars,
            _In_ ULONG length);

        // this method makes a copy of the passed in string. Ownership
        // of the passed in string remains with the caller
        HRESULT SetValue(
            _In_ const COMMON_STRING* value)
        {
            return SetValue(value->chars, value->length);
        }

        // this method makes a copy of the passed in string. Ownership
        // of the passed in string remains with the caller
        HRESULT SetValue(
            _In_ const CONST_STRING* value)
        {
            return SetValue(value->chars, value->length);
        }

        // this method makes a copy of the passed in string. Ownership
        // of the passed in string remains with the caller
        HRESULT SetValueFromString(
            __in_z_opt const WCHAR *string);

        // This method releases the ownership of the allocated buffer.
        WCHAR* Detach();

    private:
        ULONG GetCapacityFromLength(
            _In_ ULONG length);
    };
}

#endif // APPMODEL_WIDESTRING_H_
