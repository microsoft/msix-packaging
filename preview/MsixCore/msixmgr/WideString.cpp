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

HRESULT StringBuffer::InitializeFromString(
    __in_z_opt WCHAR *string)
{
    ULONG charCount = 0;    // does not include NULL terminator

    if (string != NULL)
    {
        size_t charCountSizeT = 0;
        HRESULT hr = S_OK;

        // allow for extra 1 character for the bufferLength
        RETURN_IF_FAILED(StringCchLengthW(string, StringBuffer::MaxLength - 1, &charCountSizeT));
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
    if (capacity > StringBuffer::MaxLength)
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
        RETURN_IF_FAILED(StringCchLengthW(string, StringBuffer::MaxLength - 1, &charCount));

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
