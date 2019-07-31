#include <Windows.h>
#include <WideString.hpp>
#include <StrSafe.h>

#include <WideString.hpp>
#include "GeneralUtil.hpp"
#include "ByteBuffer.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <assert.h>

using namespace MsixCoreLib;

    const COMMON_STRING String::Empty = COMMON_STRING_EMPTY;

    BOOL String::StartsWith(
        _In_reads_(stringLength) const WCHAR* string,
        _In_ const size_t stringLength,
        _In_reads_(prefixLength) const WCHAR* prefix,
        _In_ const size_t prefixLength)
    {
        if (prefixLength > stringLength)
        {
            return FALSE;
        }

        return ::memcmp(prefix, string, prefixLength * sizeof(*prefix)) == 0;
    }

    HRESULT String::CaseInsensitiveStartsWith(
        _In_ PCWSTR string,
        _In_ PCWSTR suffix,
        _Out_ BOOL* result)
    {
        size_t stringLength;
        size_t suffixLength;
        HRESULT hr;

        assert(string != NULL);
        assert(suffix != NULL);
        assert(result != NULL);

        RETURN_IF_FAILED(StringCchLengthW(string, String::MaxLength, &stringLength));
        RETURN_IF_FAILED(StringCchLengthW(suffix, String::MaxLength, &suffixLength));

        *result = CaseInsensitiveStartsWith(
            string,
            static_cast<ULONG>(stringLength),
            suffix,
            static_cast<ULONG>(suffixLength));

        return NOERROR;
    }

    BOOL String::EndsWith(
        _In_reads_(stringLength) const WCHAR* string,
        _In_ const size_t stringLength,
        _In_reads_(suffixLength) const WCHAR* suffix,
        _In_ const size_t suffixLength)
    {
        if (suffixLength > stringLength)
        {
            return FALSE;
        }

        return ::memcmp(
            string + (stringLength - suffixLength),
            suffix,
            suffixLength * sizeof(*suffix)) == 0;
    }

    BOOL String::CaseInsensitiveEndsWith(
        _In_reads_(stringLength) const WCHAR* string,
        _In_ const ULONG stringLength,
        _In_reads_(suffixLength) const WCHAR* suffix,
        _In_ const ULONG suffixLength)
    {
        if (suffixLength > stringLength)
        {
            return FALSE;
        }

        return CaseInsensitiveEquals(string + (stringLength - suffixLength), suffixLength, suffix, suffixLength);
    }

    HRESULT String::CaseInsensitiveEndsWith(
        _In_ PCWSTR string,
        _In_ PCWSTR suffix,
        _Out_ BOOL* result)
    {
        size_t stringLength;
        size_t suffixLength;
        HRESULT hr;

        assert(string != NULL);
        assert(suffix != NULL);
        assert(result != NULL);

        RETURN_IF_FAILED(StringCchLengthW(string, String::MaxLength, &stringLength));
        RETURN_IF_FAILED(StringCchLengthW(suffix, String::MaxLength, &suffixLength));

        *result = CaseInsensitiveEndsWith(
            string,
            static_cast<ULONG>(stringLength),
            suffix,
            static_cast<ULONG>(suffixLength));

        return NOERROR;
    }

    BOOL String::Contains(
        _In_ const COMMON_STRING& string,
        _In_ WCHAR ch)
    {
        WCHAR* pchMax = &string.chars[string.length];
        for (WCHAR* pch = string.chars; pch < pchMax; pch++)
        {
            if (*pch == ch)
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    HRESULT String::Replace(
        _In_ const COMMON_STRING& string,
        _In_ WCHAR searchCh,
        _In_ WCHAR replaceCh)
    {
        WCHAR* pchMax = &string.chars[string.length];
        for (WCHAR* pch = string.chars; pch < pchMax; pch++)
        {
            if (*pch == searchCh)
            {
                *pch = replaceCh;
            }
        }
        return S_OK;
    }

    HRESULT String::Clone(
        _In_ const COMMON_STRING& string,
        _Out_ COMMON_STRING* outString)
    {
        if (string.chars == NULL)
        {
            assert(string.length == 0);
            outString->chars = NULL;
            outString->length = 0;
        }
        else
        {
            assert(string.chars[string.length] == L'\0');

            // +1 for zero terminator
            ULONG byteCount;
            RETURN_IF_FAILED(AddNew(string.length, 1, &byteCount));
            RETURN_IF_FAILED(Multiply(byteCount, sizeof(WCHAR), &byteCount));

            WCHAR* chars = new APPXCOMMON_NEW_TAG WCHAR[string.length + 1];
            //RETURN_IF_NULL_ALLOC(chars);

            CopyMemory(chars, string.chars, string.length * sizeof(WCHAR));
            chars[string.length] = L'\0';
            outString->chars = chars;
            outString->length = string.length;
        }

        return S_OK;
    }

    HRESULT String::Clone(
        _In_ const COMMON_STRING& string,
        __deref_out COMMON_STRING** outString)
    {
        AutoPtr<COMMON_STRING> newString(new APPXCOMMON_NEW_TAG COMMON_STRING());
        //RETURN_IF_NULL_ALLOC(newString);
        RETURN_IF_FAILED(Clone(string, newString));
        *outString = newString.Detach();
        return S_OK;
    }

    ULONG String::GetHashCode(
        _In_ const COMMON_STRING& value)
    {
        // Hash function obtained from BCL String::GetHashCode

        // int hash1 = 5381;
        // int hash2 = hash1;
        // int     c;
        // char *s = src;
        // while ((c = s[0]) != 0) {
        //     hash1 = ((hash1 << 5) + hash1) ^ c;
        //     c = s[1];
        //     if (c == 0)
        //         break;
        //     hash2 = ((hash2 << 5) + hash2) ^ c;
        //     s += 2;
        // }
        // return hash1 + (hash2 * 1566083941);

        ULONG hash1 = 5381;
        ULONG hash2 = hash1;
        const WCHAR* pch = value.chars;
        const WCHAR* pchMax = &value.chars[value.length];
        while (pch + 2 <= pchMax)
        {
            hash1 = ((hash1 << 5) + hash1) ^ pch[0];
            hash2 = ((hash2 << 5) + hash2) ^ pch[1];
            pch += 2;
        }
        if (pch < pchMax)
        {
            hash1 = ((hash1 << 5) + hash1) ^ pch[0];
        }
        return hash1 + (hash2 * 1566083941);
    }

    BOOL String::CaseInsensitiveEquals(
        _In_reads_(length1) const WCHAR* chars1,
        _In_ ULONG length1,
        _In_reads_(length2) const WCHAR* chars2,
        _In_ ULONG length2)
    {
        if (String::Equals(chars1, length1, chars2, length2))
        {
            return TRUE;
        }
        return CaseInsensitiveCompare(chars1, length1, chars2, length2) == 0;
    }

    BOOL __success(return == TRUE) String::FindFirst(
        _In_ const COMMON_STRING& string,
        _In_ WCHAR ch,
        _Out_ ULONG* offset)
    {
        for (ULONG i = 0; i < string.length; i++)
        {
            if (string.chars[i] == ch)
            {
                *offset = i;
                return TRUE;
            }
        }

        return FALSE;
    }

    BOOL __success(return == TRUE) String::FindLast(
        _In_ const COMMON_STRING& string,
        _In_ WCHAR ch,
        _Out_ ULONG* offset)
    {
        for (LONG i = string.length - 1; i >= 0; i--)
        {
            if (string.chars[i] == ch)
            {
                *offset = i;
                return TRUE;
            }
        }

        return FALSE;
    }

    HRESULT String::InitializeCommonString(
        __in_z_opt WCHAR* inputString,
        _Out_ COMMON_STRING* commonString)
    {
        assert(commonString != NULL);

        if (inputString != NULL)
        {
            HRESULT hr;
            size_t stringLength;
            RETURN_IF_FAILED(StringCchLengthW(inputString, String::MaxLength, &stringLength));
            commonString->length = (ULONG)stringLength; // stringLength is less than MaxLength which is a ULONG.
        }
        else
        {
            commonString->length = 0;
        }

        commonString->chars = inputString;
        return NOERROR;
    }

    HRESULT StringBuffer::InitializeFromString(
        __in_z_opt WCHAR *string)
    {
        ULONG charCount = 0;    // does not include NULL terminator

        if (string != NULL)
        {
            size_t charCountSizeT = 0;
            HRESULT hr = S_OK;

            // allow for extra 1 character for the bufferLength
            RETURN_IF_FAILED(StringCchLengthW(string, String::MaxLength - 1, &charCountSizeT));
            charCount = static_cast<ULONG>(charCountSizeT);
        }

        if (this->value.chars != NULL)
        {
            delete[] this->value.chars;
            this->value.chars = NULL;
        }

        this->value.chars = string;
        this->value.length = charCount;
        this->bufferLength = charCount + 1;

        return NOERROR;
    }

    void StringBuffer::Clear()
    {
        if (this->value.chars != NULL)
        {
            assert(this->value.chars[this->value.length] == L'\0');
#pragma prefast(suppress:26018, "The buffer is still zero-terminated in the proceeding line.")
            DebugFillMemory(this->value.chars, this->bufferLength * sizeof(WCHAR));
            *this->value.chars = L'\0';
        }
        this->value.length = 0;
    }

#if defined(APPXHEAP_USE_OTHER) || defined(_UCRT)
    static HRESULT EnsureBufferCapacity(
        _In_reads_(currentCapacity) WCHAR* currentBuffer,
        _In_ ULONG currentCapacity,
        _In_ ULONG desiredCapacity,
        _Outptr_result_buffer_(desiredCapacity) WCHAR** reallocated)
    {
        if (currentCapacity < desiredCapacity)
        {
            WCHAR* newChars = new APPXCOMMON_NEW_TAG WCHAR[desiredCapacity];
            //RETURN_IF_NULL_ALLOC(newChars);
            auto err = memcpy_s(newChars, desiredCapacity * sizeof(*currentBuffer), currentBuffer, currentCapacity * sizeof(*currentBuffer));
            if (err)
            {
                delete[] newChars;
                return HRESULT_FROM_WIN32(E_INVALIDARG);
                //RETURN_HR_MSG(E_INVALIDARG, "0x%08lx", err);
            }
            delete[] currentBuffer;
            *reallocated = newChars;
        }
        else
        {
            *reallocated = currentBuffer;
        }

        return S_OK;
    }
#endif

    HRESULT StringBuffer::SetCapacity(
        _In_ ULONG capacity)
    {
        if (capacity > String::MaxLength)
        {
            return HRESULT_FROM_WIN32(ERROR_PARAMETER_QUOTA_EXCEEDED);
        }

        if (capacity == 0)
        {
            if (this->value.chars != NULL)
            {
                assert(this->value.chars[this->value.length] == L'\0');
                delete[] this->value.chars;
                this->value.chars = NULL;
                this->value.length = 0;
            }
            else
            {
                assert(this->value.length == 0);
            }

            this->bufferLength = 0;
        }
        else
        {
            if (capacity + 1 != this->bufferLength)
            {
                WCHAR* newChars;

                __analysis_assume(wcslen(this->value.chars) == this->bufferLength);

#if defined(APPXHEAP_USE_OTHER) || defined(_UCRT)
                RETURN_IF_FAILED(EnsureBufferCapacity(this->value.chars, this->bufferLength, capacity + 1, &newChars));
#else
                RETURN_IF_FAILED(HeapReAllocFunction(
                    this->value.chars,
                    this->bufferLength * sizeof(WCHAR),
                    (capacity + 1) * sizeof(WCHAR),
                    (void**)&newChars));
#endif
                this->bufferLength = capacity + 1;
                this->value.chars = newChars;

                if (this->value.length > capacity)
                {
                    this->value.length = capacity;
                    this->value.chars[capacity] = L'\0';
                }
                else if (this->value.length < capacity)
                {
                    if (this->value.length == 0)
                    {
                        *this->value.chars = L'\0';
                    }

                    DebugFillMemory(
                        &this->value.chars[this->value.length + 1],
                        (this->bufferLength - this->value.length - 1) * sizeof(WCHAR));

                    __analysis_assume_nullterminated(this->value.chars);
                    assert(this->value.chars[this->value.length] == L'\0');
                }
                else
                {
                    __analysis_assume_nullterminated(this->value.chars);
                    assert(this->value.chars[this->value.length] == L'\0');
                }
            }
        }

        return S_OK;
    }

    ULONG StringBuffer::GetCapacityFromLength(
        _In_ ULONG length)
    {
        if (length > minCapacity)
        {
            return length;
        }
        ULONG capacity = initialCapacity;
        while (capacity < length)
        {
            capacity = capacity * 2;
        }
        return capacity;
    }

    HRESULT StringBuffer::SetLength(
        _In_ ULONG length)
    {
        HRESULT hr;
        if (length > this->GetCapacity())
        {
            RETURN_IF_FAILED(SetCapacity(length));
        }
        if (this->value.length > length)
        {
            assert(this->value.chars[this->value.length] == L'\0');
            DebugFillMemory(
                &this->value.chars[length] + 1,
                (this->value.length - length - 1) * sizeof(WCHAR));
        }
        this->value.length = length;
        if (this->value.length > 0)
        {
            this->value.chars[this->value.length] = L'\0';
        }
        return NOERROR;
    }

    HRESULT StringBuffer::SetValue(
        _In_reads_(length) const WCHAR* chars,
        _In_ ULONG length)
    {
        HRESULT hr;

#if DBG
        // don't allow embedded zeros.
        for (ULONG i = 0; i < length; i++)
        {
            if (chars[i] == '\0')
            {
                return E_INVALIDARG;
            }
        }
#endif

        ULONG byteCount;
        RETURN_IF_FAILED(Multiply(length, sizeof(WCHAR), &byteCount));
        if (length > this->GetCapacity() || this->GetCapacity() > minCapacity)
        {
            RETURN_IF_FAILED(SetCapacity(GetCapacityFromLength(length)));
        }
#pragma prefast(suppress:26018, "SetCapacity() accounts for the null-terminator in addition to the length.")
        RETURN_IF_FAILED(SetLength(length));
#pragma prefast(suppress:26018, "SetLength() puts zero after the length # of characters == byteCount # of bytes.")
        ::CopyMemory(this->value.chars, chars, byteCount);
        return NOERROR;
    }

    HRESULT StringBuffer::SetValueFromString(
        __in_z_opt const WCHAR *string)
    {
        if (string == NULL)
        {
            Clear();
            return NOERROR;
        }
        else
        {
            size_t charCount = 0;
            HRESULT hr = S_OK;

            // allow for extra 1 character for the bufferLength
            RETURN_IF_FAILED(StringCchLengthW(string, String::MaxLength - 1, &charCount));

            return SetValue(string, static_cast<ULONG>(charCount));
        }
    }

    WCHAR* StringBuffer::Detach()
    {
        WCHAR* bufferPointer;

        bufferPointer = this->value.chars;
        this->value.chars = NULL;
        this->value.length = 0;
        this->bufferLength = 0;
        return bufferPointer;
    }
