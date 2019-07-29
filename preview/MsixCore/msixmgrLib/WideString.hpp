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

    class String
    {
    public:
        static const COMMON_STRING Empty;

        // Limit MaxLength to 1G - 1 (for the zero terminator),
        // so the largest physical string is 2G so math fits within a signed int32
        static const ULONG MaxLength = 0x7FFFFFFE / sizeof(WCHAR);

        inline static bool Equals(
            _In_ PCWSTR string1,
            _In_ PCWSTR string2)
        {
            return Compare(string1, string2) == 0;
        }

        inline static BOOL Equals(
            _In_reads_(length1) const WCHAR* chars1,
            _In_ ULONG length1,
            _In_reads_(length2) const WCHAR* chars2,
            _In_ ULONG length2)
        {
            return length1 == length2 && ::memcmp(chars1, chars2, length1 * sizeof(WCHAR)) == 0;
        }

        inline static BOOL Equals(
            _In_ const COMMON_STRING* string1,
            _In_ const COMMON_STRING* string2)
        {
            assert(string1 != NULL);
            assert(string2 != NULL);
            return Equals(string1->chars, string1->length, string2->chars, string2->length);
        }

        inline static BOOL Equals(
            _In_ const COMMON_STRING& string1,
            _In_ const COMMON_STRING& string2)
        {
            return Equals(string1.chars, string1.length, string2.chars, string2.length);
        }

        inline static ULONG GetHashCode(
            _In_ const COMMON_STRING* string)
        {
            return GetHashCode(*string);
        }

        static ULONG GetHashCode(
            _In_ const COMMON_STRING& string);

        inline static int Compare(
            _In_ PCWSTR string1,
            _In_ PCWSTR string2)
        {
            int value = CompareStringOrdinal(string1, -1, string2, -1, FALSE);
            assert(value != 0);
            return value - 2; // CSTR_LESS_THAN => -1, CSTR_EQUAL => 0, CSTR_GREATER_THAN => +1
        }

        inline static int CaseInsensitiveCompare(
            _In_reads_(length1) const WCHAR* chars1,
            _In_ ULONG length1,
            _In_reads_(length2) const WCHAR* chars2,
            _In_ ULONG length2)
        {
            // NOTE: CompareString will fail if passed a NULL pointer even when length is zero.
            if (length1 == 0)
            {
                chars1 = L"";
            }
            if (length2 == 0)
            {
                chars2 = L"";
            }
            int value = CompareStringOrdinal(chars1, length1, chars2, length2, TRUE);
            assert(value != 0);
            return value - 2; // CSTR_LESS_THAN => -1, CSTR_EQUAL => 0, CSTR_GREATER_THAN => +1
        }

        inline static int CaseInsensitiveCompare(
            _In_ PCWSTR string1,
            _In_ PCWSTR string2)
        {
            int value = CompareStringOrdinal(string1, -1, string2, -1, TRUE);
            assert(value != 0);
            return value - 2; // CSTR_LESS_THAN => -1, CSTR_EQUAL => 0, CSTR_GREATER_THAN => +1
        }

        inline static int CaseInsensitiveCompare(
            _In_ const COMMON_STRING* string1,
            _In_ const COMMON_STRING* string2)
        {
            assert(string1 != NULL);
            assert(string2 != NULL);
            return CaseInsensitiveCompare(string1->chars, string1->length, string2->chars, string2->length);
        }

        inline static int CaseInsensitiveCompare(
            _In_ const COMMON_STRING& string1,
            _In_ const COMMON_STRING& string2)
        {
            return CaseInsensitiveCompare(string1.chars, string1.length, string2.chars, string2.length);
        }

        static BOOL CaseInsensitiveEquals(
            _In_reads_(length1) const WCHAR* chars1,
            _In_ ULONG length1,
            _In_reads_(length2) const WCHAR* chars2,
            _In_ ULONG length2);

        inline static BOOL CaseInsensitiveEquals(
            _In_ const COMMON_STRING* string1,
            _In_ const COMMON_STRING* string2)
        {
            assert(string1 != NULL);
            assert(string2 != NULL);
            return CaseInsensitiveEquals(string1->chars, string1->length, string2->chars, string2->length);
        }

        inline static BOOL CaseInsensitiveEquals(
            _In_ const COMMON_STRING& string1,
            _In_ const COMMON_STRING& string2)
        {
            return CaseInsensitiveEquals(string1.chars, string1.length, string2.chars, string2.length);
        }

        /// Returns true if 2 null-terminated strings are equal, ignoring case.
        inline static BOOL CaseInsensitiveEquals(
            _In_ PCWSTR string1,
            _In_ PCWSTR string2)
        {
            return (CaseInsensitiveCompare(string1, string2) == 0);
        }

        inline static BOOL CaseInsensitiveIsSubString(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ ULONG stringLength,
            _In_reads_(subStringLength) const WCHAR* subString,
            _In_ ULONG subStringLength)
        {
            return (CaseInsensitiveGetSubString(string, stringLength, subString, subStringLength) != nullptr);
        }

        inline static PCWSTR CaseInsensitiveGetSubString(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ ULONG stringLength,
            _In_reads_(subStringLength) const WCHAR* subString,
            _In_ ULONG subStringLength)
        {
            if (stringLength < subStringLength)
            {
                return nullptr;
            }

            for (ULONG startIndex = 0; startIndex <= (stringLength - subStringLength); startIndex++)
            {
                if (CaseInsensitiveEquals(&string[startIndex], subStringLength, subString, subStringLength))
                {
                    return &string[startIndex];
                }
            }

            return nullptr;
        }

        inline static BOOL CaseInsensitiveIsSubString(
            _In_ const COMMON_STRING* string,
            _In_ const COMMON_STRING* subString)
        {
            assert(string != NULL);
            assert(subString != NULL);
            return CaseInsensitiveIsSubString(string->chars, string->length, subString->chars, subString->length);
        }

        inline static BOOL CaseInsensitiveIsSubString(
            _In_ const COMMON_STRING& string,
            _In_ const COMMON_STRING& subString)
        {
            return CaseInsensitiveIsSubString(string.chars, string.length, subString.chars, subString.length);
        }

        /// Zero out the character content of the string.  The length & the chars pointer are unchanged.
        inline static void Erase(
            _In_ COMMON_STRING* string)
        {
            assert(string != NULL);
            Erase(*string);
        }

        inline static void Erase(
            _In_ COMMON_STRING& string)
        {
            assert((string.length == 0u) || (string.chars != NULL));
            SecureZeroMemory(string.chars, string.length * sizeof(WCHAR));
        }

        inline static void ToLowerAscii(
            _In_reads_(stringLength) WCHAR* string,
            _In_ ULONG stringLength)
        {
            for (ULONG i = 0; i < stringLength; i++)
            {
                if (string[i] >= L'A' && string[i] <= L'Z')
                {
                    string[i] = string[i] - L'A' + L'a';
                }
            }
        }

        static BOOL StartsWith(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ const size_t stringLength,
            _In_reads_(prefixLength) const WCHAR* prefix,
            _In_ const size_t prefixLength);

        inline static BOOL CaseInsensitiveStartsWith(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ const ULONG stringLength,
            _In_reads_(prefixLength) const WCHAR* prefix,
            _In_ const ULONG prefixLength)
        {
            return (stringLength >= prefixLength) &&
                (CaseInsensitiveCompare(string, prefixLength, prefix, prefixLength) == 0);
        }

        inline static BOOL StartsWith(
            _In_ PCWSTR string,
            _In_ PCWSTR prefix)
        {
            assert(string != NULL);
            assert(prefix != NULL);
            return StartsWith(string, wcslen(string), prefix, wcslen(prefix));
        }

        static HRESULT CaseInsensitiveStartsWith(
            _In_ PCWSTR string,
            _In_ PCWSTR prefix,
            _Out_ BOOL* result);

        inline static BOOL StartsWith(
            _In_ const COMMON_STRING* string,
            _In_ const COMMON_STRING* prefix)
        {
            assert(string != NULL);
            assert(prefix != NULL);
            return StartsWith(string->chars, string->length, prefix->chars, prefix->length);
        }

        inline static BOOL CaseInsensitiveStartsWith(
            _In_ const COMMON_STRING* string,
            _In_ const COMMON_STRING* prefix)
        {
            assert(string != NULL);
            assert(prefix != NULL);
            return CaseInsensitiveStartsWith(string->chars, string->length, prefix->chars, prefix->length);
        }

        inline static BOOL StartsWith(
            _In_ const COMMON_STRING& string,
            _In_ const COMMON_STRING& prefix)
        {
            return StartsWith(string.chars, string.length, prefix.chars, prefix.length);
        }

        inline static BOOL CaseInsensitiveStartsWith(
            _In_ const COMMON_STRING& string,
            _In_ const COMMON_STRING& prefix)
        {
            return CaseInsensitiveStartsWith(string.chars, string.length, prefix.chars, prefix.length);
        }

        static BOOL EndsWith(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ const size_t stringLength,
            _In_reads_(suffixLength) const WCHAR* suffix,
            _In_ const size_t suffixLength);

        static BOOL CaseInsensitiveEndsWith(
            _In_reads_(stringLength) const WCHAR* string,
            _In_ const ULONG stringLength,
            _In_reads_(suffixLength) const WCHAR* suffix,
            _In_ const ULONG suffixLength);

        inline static BOOL EndsWith(
            _In_ PCWSTR string,
            _In_ PCWSTR suffix)
        {
            assert(string != NULL);
            assert(suffix != NULL);
            return EndsWith(string, wcslen(string), suffix, wcslen(suffix));
        }

        static HRESULT CaseInsensitiveEndsWith(
            _In_ PCWSTR string,
            _In_ PCWSTR suffix,
            _Out_ BOOL* result);

        inline static BOOL EndsWith(
            _In_ const COMMON_STRING* string,
            _In_ const COMMON_STRING* suffix)
        {
            assert(string != NULL);
            assert(suffix != NULL);
            return EndsWith(string->chars, string->length, suffix->chars, suffix->length);
        }

        inline static BOOL CaseInsensitiveEndsWith(
            _In_ const COMMON_STRING* string,
            _In_ const COMMON_STRING* suffix)
        {
            assert(string != NULL);
            assert(suffix != NULL);
            return CaseInsensitiveEndsWith(string->chars, string->length, suffix->chars, suffix->length);
        }

        inline static BOOL EndsWith(
            _In_ const COMMON_STRING& string,
            _In_ const COMMON_STRING& suffix)
        {
            return EndsWith(string.chars, string.length, suffix.chars, suffix.length);
        }

        inline static BOOL CaseInsensitiveEndsWith(
            _In_ const COMMON_STRING& string,
            _In_ const COMMON_STRING& suffix)
        {
            return CaseInsensitiveEndsWith(string.chars, string.length, suffix.chars, suffix.length);
        }

        inline static BOOL Contains(
            _In_ const COMMON_STRING* string,
            _In_ WCHAR ch)
        {
            assert(string != NULL);
            return Contains(*string, ch);
        }

        static BOOL Contains(
            _In_ const COMMON_STRING& string,
            _In_ WCHAR ch);

        inline static HRESULT Replace(
            _In_ const COMMON_STRING* string,
            _In_ WCHAR searchCh,
            _In_ WCHAR replaceCh)
        {
            assert(string != NULL);
            return Replace(*string, searchCh, replaceCh);
        }

        static HRESULT Replace(
            _In_ const COMMON_STRING& string,
            _In_ WCHAR searchCh,
            _In_ WCHAR replaceCh);

        inline static HRESULT Clone(
            _In_ const COMMON_STRING* string,
            _Out_ COMMON_STRING* outString)
        {
            assert(string != NULL);
            assert(outString != NULL);
            return Clone(*string, outString);
        }

        static HRESULT Clone(
            _In_ const COMMON_STRING& string,
            _Out_ COMMON_STRING* outString);

        inline static HRESULT Clone(
            _In_ const COMMON_STRING* string,
            __deref_out COMMON_STRING** outString)
        {
            assert(string != NULL);
            assert(outString != NULL);
            return Clone(*string, outString);
        }

        static HRESULT Clone(
            _In_ const COMMON_STRING& string,
            __deref_out COMMON_STRING** outString);

        inline static __success(return == TRUE) BOOL FindFirst(
            _In_ const COMMON_STRING* string,
            _In_ WCHAR ch,
            __deref_out_range(0, string->length - 1) ULONG* offset)
        {
            assert(string != NULL);
            assert(offset != NULL);
            return FindFirst(*string, ch, offset);
        }

        static __success(return == TRUE) BOOL FindFirst(
            _In_ const COMMON_STRING& string,
            _In_ WCHAR ch,
            __deref_out_range(0, string.length - 1)  ULONG* offset);

        inline static __success(return == TRUE) BOOL FindLast(
            _In_ const COMMON_STRING* string,
            _In_ WCHAR ch,
            __deref_out_range(0, string->length - 1) ULONG* offset)
        {
            assert(string != NULL);
            assert(offset != NULL);
            return FindLast(*string, ch, offset);
        }

        static __success(return == TRUE) BOOL FindLast(
            _In_ const COMMON_STRING& string,
            _In_ WCHAR ch,
            __deref_out_range(0, string.length - 1) ULONG* offset);

        static HRESULT InitializeCommonString(
            __in_z_opt WCHAR* inputString,
            _Out_ COMMON_STRING* commonString);
    };

    class StringBuffer
    {
    private:
        COMMON_STRING value = {}; // value.length does not include terminating NULL
        ULONG bufferLength = 0; // bufferLength includes space for the terminating NULL
        static const ULONG minCapacity = 32;
        static const ULONG initialCapacity = 8;

    public:
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

        // Zero out the character content of the string.  The capacity, the length and the char pointer are unchanged.
        void Erase()
        {
            String::Erase(&value);
        }

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

        inline static BOOL CaseInsensitiveEquals(
            _In_ const StringBuffer* stringBuffer1,
            _In_ const StringBuffer* stringBuffer2)
        {
            return String::CaseInsensitiveEquals(stringBuffer1->value.chars, stringBuffer1->value.length, stringBuffer2->value.chars, stringBuffer2->value.length);
        }

        inline static int CaseInsensitiveCompare(
            _In_ const StringBuffer* stringBuffer1,
            _In_ const StringBuffer* stringBuffer2)
        {
            return String::CaseInsensitiveCompare(stringBuffer1->value.chars, stringBuffer1->value.length, stringBuffer2->value.chars, stringBuffer2->value.length);
        }

        // This method releases the ownership of the allocated buffer.
        WCHAR* Detach();

    private:
        ULONG GetCapacityFromLength(
            _In_ ULONG length);
    };
}

#endif // APPMODEL_WIDESTRING_H_
