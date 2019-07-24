
#pragma once

#include <assert.h>

#define APPXCOMMON_NEW_TAG (std::nothrow)
#define DebugFillMemory(pv, cb)

namespace MsixCoreLib
{

    struct COMMON_BYTES
    {
        ULONG length;
        __field_ecount(length) BYTE* bytes;
    };

    class ByteBuffer
    {
    private:
        COMMON_BYTES value;
        ULONG capacity;

        static const ULONG minCapacity = 32;
        static const ULONG initialCapacity = 8;

        ULONG GetCapacityFromLength(
            _In_ ULONG length);

    public:
        static const ULONG MaxLength = 0x7FFFFFFF;  // Limit MaxLength to 2G-1, so math fits within a signed int32

        ByteBuffer()
        {
            this->value.bytes = nullptr;
            this->value.length = 0;
            this->capacity = 0;
        }

        ~ByteBuffer()
        {
            delete this->value.bytes;
        }

        const COMMON_BYTES* GetBytes()
        {
            return &value;
        }

        BYTE* GetBufferPtr()
        {
            return value.bytes;
        }

        const BYTE* GetBufferPtr() const
        {
            return value.bytes;
        }

        ULONG GetLength() const
        {
            return value.length;
        }

        ULONG GetCapacity() const
        {
            return capacity;
        }

        BYTE* GetRange(
            _In_ ULONG offset,
            _In_ ULONG count)
        {
            UNREFERENCED_PARAMETER(count); // Only used on debug builds.

            assert(
                offset < this->value.length &&
                count <= this->value.length && offset <= this->value.length - count);

            return &value.bytes[offset];
        }

        void Clear();

        // Zero out the content of the byte buffer.  The capacity, the length and the byte pointer are unchanged.
        void Erase()
        {
            Erase(&this->value);
        }

        // returns buffer to original empty state
        void Reset()
        {
            delete[] this->value.bytes;
            this->value.bytes = nullptr;
            this->value.length = 0;
            this->capacity = 0;
        }

        void Reverse()
        {
            Reverse(&this->value);
        }

        HRESULT SetLength(
            _In_ ULONG length);

        HRESULT SetCapacity(
            _In_ ULONG capacity);

        HRESULT SetValue(
            _In_reads_(length) const BYTE* bytes,
            _In_ ULONG length);

        HRESULT SetValue(
            _In_ const COMMON_BYTES* value)
        {
            return SetValue(value->bytes, value->length);
        }

        static BOOL Equals(
            _In_reads_(length1) const BYTE* bytes1,
            _In_ ULONG length1,
            _In_reads_(length2) const BYTE* bytes2,
            _In_ ULONG length2)
        {
            return length1 == length2 && ::memcmp(bytes1, bytes2, length1) == 0;
        }

        static BOOL Equals(
            _In_ const COMMON_BYTES* bytes1,
            _In_ const COMMON_BYTES* bytes2)
        {
            assert(bytes1 != nullptr && bytes2 != nullptr);
            return Equals(bytes1->bytes, bytes1->length, bytes2->bytes, bytes2->length);
        }

        // Zero out the content of the BYTES. The length & the bytes pointer are unchanged.
        static void Erase(
            _In_ COMMON_BYTES* bytes)
        {
            ::SecureZeroMemory(bytes->bytes, bytes->length);
        }

        static void Reverse(
            _In_ COMMON_BYTES* bytes)
        {
            BYTE* low = &bytes->bytes[0];
            BYTE* high = &bytes->bytes[bytes->length];
            for (high--; low < high; low++, high--)
            {
                BYTE tmp = *low;
                *low = *high;
                *high = tmp;
            }
        }
    };
}
