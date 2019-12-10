#pragma once

#include "../GeneralUtil.hpp"

#define DebugFillMemory(pv, cb)

namespace MsixCoreLib
{
    class Base32Encoding
    {
    public:

        static const ULONG MaxValue = 0xFFFFFFFF;

        static HRESULT GetCharCount(
            _In_ ULONG byteCount,
            _Out_ ULONG* outWCharCount);

        // The resulting string is not NULL terminated and
        // the base32 string produced does not use trailing padding characters.
        // This means that the result is not decodable unless the original number
        // of bytes is known.
        static HRESULT GetChars(
            _In_reads_(byteCount) const BYTE* bytes,
            _In_ ULONG byteCount,
            _In_ ULONG maxWCharCount,
            __out_ecount_part_opt(maxWCharCount, *outWCharCount) WCHAR* wchars,
            __out_range(0, maxWCharCount) ULONG* outWCharCount);

    private:
        static WCHAR ValueToDigit(
            _In_ BYTE value);

        static HRESULT Multiply(
            _In_ ULONG d1,
            _In_ ULONG d2,
            _Out_ ULONG *result);
    };
}
