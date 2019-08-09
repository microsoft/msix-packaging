#include <Windows.h>
#include <Base32Encoding.hpp>

#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <assert.h>

namespace MsixCoreLib
{
    HRESULT Base32Encoding::GetCharCount(
        _In_ ULONG byteCount,
        _Out_ ULONG* outWCharCount)
    {
        HRESULT hr = S_OK;

        assert(outWCharCount != NULL);

        if (byteCount != 0)
        {
            static const ULONG bitsPerByte = 8;
            static const ULONG bitsPerBase32Digit = 5;
            ULONG bitCount;

            RETURN_IF_FAILED(Multiply(byteCount, bitsPerByte, &bitCount));

            *outWCharCount = bitCount / bitsPerBase32Digit;

            // We need to round up the division.  When there aren't exactly enough bits to fill in the final base32 digit,
            // the remaining bits are set to zero.
            if (bitCount % bitsPerBase32Digit > 0)
            {
                ++*outWCharCount;
            }
        }
        else
        {
            *outWCharCount = 0;
        }

        return hr;
    }

    HRESULT Base32Encoding::GetChars(
        _In_reads_(byteCount) const BYTE* bytes,
        _In_ ULONG byteCount,
        _In_ ULONG maxWCharCount,
        __out_ecount_part_opt(maxWCharCount, *outWCharCount) WCHAR* wchars,
        __out_range(0, maxWCharCount) ULONG* outWCharCount)
    {
        HRESULT hr = S_OK;

        assert(bytes != NULL);
        assert(wchars != NULL);
        assert(outWCharCount != NULL);

        ULONG wcharCount = 0;
        ULONG wcharsIdx = 0;

        RETURN_IF_FAILED(GetCharCount(byteCount, &wcharCount));
        if (wcharCount > maxWCharCount)
        {
            return HRESULT_FROM_WIN32(E_INVALIDARG);
        }

        // Consider groups of five bytes.  This is the smallest number of bytes that has a number of bits
        // that's evently divisible by five.
        // Every five bits starting with the most significant of the first byte are made into a base32 value.
        // Each value is used to index into the alphabet array to produce a base32 digit.
        // When out of bytes but the corresponding base32 value doesn't yet have five bits, 0 is used.
        // Normally in these cases a particular number of '=' characters are appended to the resulting base32
        // string to indicate how many bits didn't come from the actual byte value.  For our purposes no
        // such padding characters are necessary.
        //
        // Bytes:         aaaaaaaa  bbbbbbbb  cccccccc  dddddddd  eeeeeeee
        // Base32 Values: 000aaaaa  000aaabb  000bbbbb  000bcccc  000ccccd  000ddddd 000ddeee 000eeeee
        //
        // Combo of byte    a & F8    a & 07    b & 3E    b & 01    c & 0F    d & 7C   d & 03   e & 1F
        // values except              b & C0              c & F0    d & 80             e & E0
        // for shifting

        // Make sure the following math doesn't overflow.
        if (maxWCharCount > MaxValue / sizeof(*wchars))
        {
            return HRESULT_FROM_WIN32(E_INVALIDARG);
        }

        DebugFillMemory(wchars, maxWCharCount * sizeof(*wchars));

        for (ULONG byteIdx = 0; byteIdx < byteCount; byteIdx += 5)
        {
            BYTE firstByte = bytes[byteIdx];
            BYTE secondByte = (byteIdx + 1) < byteCount ? bytes[byteIdx + 1] : 0;

            __assume(wcharsIdx + 2 <= wcharCount);
            wchars[wcharsIdx++] = ValueToDigit((firstByte & 0xF8) >> 3);
            wchars[wcharsIdx++] = ValueToDigit(
                ((firstByte & 0x07) << 2) | ((secondByte & 0xC0) >> 6));

            if (byteIdx + 1 < byteCount)
            {
                BYTE thirdByte = (byteIdx + 2) < byteCount ? bytes[byteIdx + 2] : 0;

                __assume(wcharsIdx + 2 <= wcharCount);
                wchars[wcharsIdx++] = ValueToDigit((secondByte & 0x3E) >> 1);
                wchars[wcharsIdx++] = ValueToDigit(
                    ((secondByte & 0x01) << 4) | ((thirdByte & 0xF0) >> 4));

                if (byteIdx + 2 < byteCount)
                {
                    BYTE fourthByte = (byteIdx + 3) < byteCount ? bytes[byteIdx + 3] : 0;

                    __assume(wcharsIdx + 1 <= wcharCount);
                    wchars[wcharsIdx++] = ValueToDigit(
                        ((thirdByte & 0x0F) << 1) | ((fourthByte & 0x80) >> 7));

                    if (byteIdx + 3 < byteCount)
                    {
                        BYTE fifthByte = (byteIdx + 4) < byteCount ? bytes[byteIdx + 4] : 0;

                        __assume(wcharsIdx + 2 <= wcharCount);
                        wchars[wcharsIdx++] = ValueToDigit((fourthByte & 0x7C) >> 2);
                        wchars[wcharsIdx++] = ValueToDigit(
                            ((fourthByte & 0x03) << 3) | ((fifthByte & 0xE0) >> 5));

                        if (byteIdx + 4 < byteCount)
                        {
                            __assume(wcharsIdx + 1 <= wcharCount);
                            wchars[wcharsIdx++] = ValueToDigit(fifthByte & 0x1F);
                        }
                    }
                }
            }
        }

        *outWCharCount = wcharCount;

        return hr;
    }

    WCHAR Base32Encoding::ValueToDigit(
        _In_ BYTE value)
    {
        assert(value < 0x20);
        // Douglas Crockford's base 32 alphabet variant is 0-9, A-Z except for i, l, o, and u.
        static const WCHAR base32DigitList[] = L"0123456789abcdefghjkmnpqrstvwxyz";
        C_ASSERT(ARRAYSIZE(base32DigitList) == 0x20 + 1); // Plus one due to NULL terminator
        return base32DigitList[value];
    }

    HRESULT Base32Encoding::Multiply(ULONG d1, ULONG d2, ULONG * result)
    {
        (*result) = d1 * d2;
        if ((d2 == 0 || d1 <= MaxValue / d2))
        {
            return NOERROR;
        }
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }
}
