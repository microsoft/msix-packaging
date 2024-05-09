//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <cstdint>

#include "Encoding.hpp"
#include "Exceptions.hpp"
#include "UnicodeConversion.hpp"

namespace MSIX { namespace Encoding {

    const std::size_t PercentageEncodingTableSize = 0x7F;
    const std::array<const wchar_t*, PercentageEncodingTableSize> PercentageEncoding =
    {   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        L"%20",  L"%21",  nullptr, L"%23",  L"%24",  L"%25",  L"%26",  L"%27",   // [space] ! # $ % & '
        L"%28",  L"%29",  nullptr, L"%2B",  L"%2C",  nullptr, nullptr, nullptr, // ( ) + ,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, L"%3B",  nullptr, L"%3D",  nullptr, nullptr, // ; =
        L"%40",  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // @
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, L"%5B",  nullptr, L"%5D",  L"%5E",  nullptr, // [ ] ^
        L"%60",  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // `
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, L"%7B",  nullptr, L"%7D",  nullptr,          // { }
    };

    struct EncodingChar
    {
        const wchar_t* encode;
        wchar_t        decode;

        bool operator==(const std::wstring& rhs) const {
            return rhs == encode;
        }
        EncodingChar(const wchar_t* e, wchar_t d) : encode(e), decode(d) {}
    };

    // Returns the file name percentage encoded.
    std::string EncodeFileName(const std::string& fileName)
    {
        ThrowErrorIf(Error::InvalidParameter, fileName.empty(), "Empty value tries to be encoded");
        std::wstring fileNameW = utf8_to_wstring(fileName);
        std::wstring result = L"";

        for (std::uint32_t index = 0; index < fileNameW.length(); index++)
        {
            std::uint32_t codepoint = static_cast<std::uint32_t>(fileNameW[index]);

            // Start of double wchar UTF-16 sequence
            if ((codepoint & 0xFC00) == 0xD800)
            {
                if ((fileNameW[index] & 0xFC00) == 0xD800 &&
                    (fileNameW[index+1] & 0xFC00) == 0xDC00)
                {
                    codepoint = (((fileNameW[index] & 0x03C0) + 0x0040) | (fileNameW[index] & 0x003F)) << 10;
                    codepoint |= (fileNameW[index+1] & 0x03FF);
                }
                else
                {
                    ThrowError(Error::InvalidParameter);
                }
                index++;
            }
            else if ((codepoint & 0xFC00) == 0xDC00)
            {
                ThrowErrorAndLog(Error::InvalidParameter, "The second surrogate pair may not exist alone");
            }

            // See if it's one of the special cases we encode
            if (codepoint < PercentageEncodingTableSize && PercentageEncoding[codepoint] != nullptr)
            {   result += PercentageEncoding[codepoint];
            }
            else if (fileNameW[index] == '\\') // replace backslash
            {   result.push_back('/');
            }
            else if (codepoint > PercentageEncodingTableSize)
            {   // Returns the length of the UTF-8 byte sequence associated with the given codepoint
                // We already know is > 0x7F, so it can't be 1 byte
                std::uint8_t totalBytes = 0;
                if (codepoint <= 0x07FF) { totalBytes = 2; }
                else if (codepoint <= 0xFFFF) { totalBytes = 3; }
                else { totalBytes = 4; }

                const std::wstring hexadecimal = L"0123456789ABCDEF";
                for (size_t byteIndex = 0; byteIndex < totalBytes; byteIndex++)
                {
                    std::uint32_t mychar;
                    switch (totalBytes - byteIndex)
                    {
                    case 1:
                        if (totalBytes == 1) { mychar = codepoint; } 
                        else { mychar = 0x80 | (codepoint & 0x003F); }
                        break;
                    case 2:
                        if (totalBytes == 2) { mychar = 0xC0 | ((codepoint & 0x07C0) >> 6); } 
                        else { mychar = 0x80 | ((codepoint & 0x0FC0) >> 6); }
                        break;
                    case 3:
                        if (totalBytes == 3) { mychar = 0xE0 | ((codepoint & 0xF000) >> 12); } 
                        else { mychar = 0x80 | ((codepoint & 0x03F000) >> 12); }
                        break;
                    case 4:
                        mychar = 0xF0 | ((codepoint & 0x1C0000) >> 18);
                        break;
                    default:
                        ThrowError(Error::Unexpected); // This should never happen.
                        break;
                    }

                    auto highDigit = mychar / hexadecimal.size();
                    auto lowDigit = mychar % hexadecimal.size();

                    ThrowErrorIf(Error::InvalidParameter, (highDigit > hexadecimal.size() || lowDigit  > hexadecimal.size()), "Invalid");
                    result.push_back('%'); // we are percentage encoding
                    result.push_back(hexadecimal[highDigit]);
                    result.push_back(hexadecimal[lowDigit]);
                }
            }
            else
            {   result.push_back(fileNameW[index]);
            }
        }
        return wstring_to_utf8(result);
    }

    const EncodingChar EncodingToChar[] =
    {   EncodingChar(L"20", ' '), EncodingChar(L"21", '!'), EncodingChar(L"23", '#'),  EncodingChar(L"24", '$'),
        EncodingChar(L"25", '%'), EncodingChar(L"26", '&'), EncodingChar(L"27", '\''), EncodingChar(L"28", '('),
        EncodingChar(L"29", ')'), EncodingChar(L"2B", '+'), EncodingChar(L"2C", ','),  EncodingChar(L"3B", ';'),
        EncodingChar(L"3D", '='), EncodingChar(L"40", '@'), EncodingChar(L"5B", '['),  EncodingChar(L"5D", ']'),
        EncodingChar(L"5E", '^'), EncodingChar(L"60", '`'), EncodingChar(L"7B", '{'),  EncodingChar(L"7D", '}')
    };

    // Convert a single hex digit to its corresponding value
    inline std::uint32_t ConvertHex(wchar_t ch)
    {
        if (ch >= '0' && ch <= '9') { return ch - '0'; }
        else if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
        else if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
        ThrowErrorAndLog(Error::Unexpected, "Invalid hexadecimal");
    }

    //+----------------------------------------------------------------------------
    //                   STRING UTF-8 PERCENT-ENCODING UTILITIES
    //
    //    Two write the UTF-8, UTF-16, and UTF-32 conversion code see the following tables from:
    //         <http://www.unicode.org/versions/Unicode4.0.0/ch03.pdf#G7404>
    //
    //    UTF-8 Bit Distribution
    //    | Unicode Codepoint          | 1st Byte | 2nd Byte | 3rd Byte | 4th Byte |
    //    |----------------------------|----------|----------|----------|----------|
    //    |          00000000 0xxxxxxx | 0xxxxxxx |          |          |          |
    //    |          00000yyy yyxxxxxx | 110yyyyy | 10xxxxxx |          |          |
    //    |          zzzzyyyy yyxxxxxx | 1110zzzz | 10yyyyyy | 10xxxxxx |          |
    //    | 000uuuuu zzzzyyyy yyxxxxxx | 11110uuu | 10uuzzzz | 10yyyyyy | 10xxxxxx |
    //
    //    Well-Formed UTF-8 Byte Sequences
    //    | Codepoint Range            | 1st Byte | 2nd Byte | 3rd Byte | 4th Byte |
    //    |----------------------------|----------|----------|----------|----------|
    //    | U+0000  ..U+007F           | 00..7F   |          |          |          |
    //    | U+0080  ..U+07FF           | C2..DF   | 80..BF   |          |          |
    //    | U+0800  ..U+0FFF           | E0       | A0..BF   | 80..BF   |          |
    //    | U+1000  ..U+CFFF           | E1..EC   | 80..BF   | 80..BF   |          |
    //    | U+D000  ..U+D7FF           | ED       | 80..9F   | 80..BF   |          |
    //    | U+E000  ..U+FFFF           | EE..EF   | 80..BF   | 80..BF   |          |
    //    | U+10000 ..U+3FFFF          | F0       | 90..BF   | 80..BF   | 80..BF   |
    //    | U+40000 ..U+FFFFF          | F1..F3   | 80..BF   | 80..BF   | 80..BF   |
    //    | U+100000..U+10FFFF         | F4       | 80..8F   | 80..BF   | 80..BF   |
    //
    //    UTF-16 Bit Distribution
    //    |     Unicode Codepoint      | UTF-16                              |
    //    |----------------------------|-------------------------------------|
    //    |          xxxxxxxx,xxxxxxxx |                   xxxxxxxx,xxxxxxxx |
    //    | 000uuuuu xxxxxxxx,xxxxxxxx | 110110ww,wwxxxxxx 110111xx,xxxxxxxx |
    //    where wwww = uuuuu - 1
    //
    //-----------------------------------------------------------------------------
    void ValidateCodepoint(std::uint32_t codepoint, std::uint32_t sequenceSize)
    {
        // The valid range of Unicode code points is [U+0000, U+10FFFF]. DecodeFileName cannot generate a value larger
        // than 0x10FFFF: The "4 Byte sequence" section of code is responsible for the most significant change to the
        // code point. The bottom 3 bits of the first byte of the four byte sequence are shifted 18 to make up the most
        // significant 3 bits of the 21 bit code point. Since the first byte is less than 0xF4 and since its most
        // significant bits must be 1111,0xxx, the only possible values are [0xF0, 0xF4].  Assuming the largest value,
        // when shifted left 18bits this gives a code point in binary of 1,00yy,yyyy,yyyy,yyyy,yyyy (where y are binary
        // digits yet to be determined by the subsequent trail bytes) so we must ensure that the next trail byte in the
        // 4 byte sequence cannot place 1's in the most significant two y's. By the above, in the case that the first
        // byte is less than 0xF4, the most significant bit is 0 and so the codepoint is at most 0x0FFFFF, so we'll 
        // only consider the case where the first byte is 0xF4.  In that case maxNextSequenceValue is set to 0x8F and
        // the min value is 0x80. Since the bottom six bits of the trail bytes are used (see 'decoded & 0x3F')
        // and the range only allows the bottom four of those bits to be set, the most significant two y's from above
        // will always be zero. Accordingly, the largest value one could produce is 0x10FFFF.
        ThrowErrorIf(Error::UnknownFileNameEncoding, codepoint >= 0x110000, "Codepoint is invalid because is too big!");

        // The range [U+D800,U+DBFF] is for high surrogates and [U+DC00,U+DFFF] is for low surrogates, neither of which 
        // a UTF-8 sequence is allowed to decode to. This function cannot generate values in that range: The range of 
        // surrogate codepoints requires 16 bits to represent and so would require a 3 byte UTF8 sequence (which 
        // represents codepoints between 0x800 and 0xFFFF). The 3 byte UTF8 sequence is of the form 
        // 1110,yyyy 10yy,yyyy 10yy,yyyy where the 'y's are the bits from the codepoint. Since in the
        // surrogate range the most significant 5 bits of the 16bit codepoint are always set, the lead byte is always
        // 1110,1101 (ED). This is a valid lead byte but the maximum for the next trail byte, specifically for this case
        // (see 'decoded == 0xED') is set to 0x9F. Since the top 5 bits are set the trail byte will be 101y,yyyy or at
        // least 0xA0 which is greater than the maximum.
        ThrowErrorIf(Error::UnknownFileNameEncoding, codepoint >= 0xD800 && codepoint <= 0xDFFF, "Invalid codepoint");
    }

    // Decodes a percentage encoded string
    std::string DecodeFileName(const std::string& fileName)
    {
        std::wstring fileNameW = utf8_to_wstring(fileName);
        std::wstring result = L"";
        for (std::uint32_t index = 0; index < fileNameW.length(); index++)
        {
            if(fileName[index] == '%')
            {
                ThrowErrorIf(Error::UnknownFileNameEncoding, index+2 >= fileNameW.length(), "Invalid encoding.")
                auto encoding = fileNameW.substr(index+1, 2);
                const auto& found = std::find(std::begin(EncodingToChar), std::end(EncodingToChar), fileNameW.substr(index+1, 2));
                if (found != std::end(EncodingToChar))
                {   // Use special cheat table
                    result.push_back(found->decode);
                    index += 2;
                }
                else
                {
                    std::uint32_t codepoint = 0;
                    std::uint32_t sequenceSize = 1;
                    std::uint32_t sequenceIndex = 0;
                    std::uint8_t minNextSequenceValue = 0;
                    std::uint8_t maxNextSequenceValue = 0;
                    bool done = false;

                    // Ok, here we go...
                    while (index < fileNameW.length() && !done)
                    {
                        if (fileNameW[index] == '%')
                        {
                            ThrowErrorIf(Error::UnknownFileNameEncoding,  index+2 >= fileNameW.length(), "Invalid encoding");

                            auto decoded = ConvertHex(fileNameW[++index]) * 16; // hi nibble
                            decoded += ConvertHex(fileNameW[++index]); // low nibble

                            if (sequenceIndex == 0)
                            {
                                if (decoded <= 0x7F)
                                {   // Actually, because of EncodingToChar, 0x7F is the only case here. <= just in case...
                                    codepoint = decoded;
                                    done = true;
                                }
                                else if (decoded >= 0xC2 && decoded <= 0xF4)
                                {   // decode and reset values for UTF-8 sequence
                                    if ((decoded & 0xE0) == 0xC0) // 2 Byte sequence starts with 110y,yyyy
                                    {
                                        sequenceSize = 2;
                                        codepoint = (decoded & 0x1F) << 6;
                                        minNextSequenceValue = 0x80;
                                        maxNextSequenceValue = 0xBF;
                                    }
                                    else if ((decoded & 0xF0) == 0xE0) // 3 Byte sequence starts with 1110,zzzz
                                    {
                                        sequenceSize = 3;
                                        codepoint = (decoded & 0x0F) << 12;
                                        minNextSequenceValue = (decoded == 0xE0 ? 0xA0 : 0x80);
                                        maxNextSequenceValue = (decoded == 0xED ? 0x9F : 0xBF);
                                    }
                                    else if ((decoded & 0xF8) == 0xF0) // 4 Byte sequence starts with 1111,0uuu
                                    {
                                        sequenceSize = 4;
                                        codepoint = ((decoded & 0x07) << 18);
                                        minNextSequenceValue = (decoded == 0xF0 ? 0x90 : 0x80);
                                        maxNextSequenceValue = (decoded == 0xF4 ? 0x8F : 0xBF);
                                    }
                                    else { ThrowError(Error::UnknownFileNameEncoding); }
                                }
                                else { ThrowError(Error::UnknownFileNameEncoding); }
                            }
                            else
                            {   // continue UTF-8 sequence
                                if (decoded >= minNextSequenceValue && decoded <= maxNextSequenceValue)
                                {   // Adjust codepoint with new bits. Trailing bytes can only contain 6 bits of information
                                    std::uint32_t shiftDistance = ((sequenceSize - sequenceIndex) - 1) * 6;
                                    codepoint |= (decoded & 0x3F) << shiftDistance;

                                    // Set values for next byte in sequence
                                    minNextSequenceValue = 0x80;
                                    maxNextSequenceValue = 0xBF;

                                    // If full sequence then we're done!
                                    if (sequenceSize == sequenceIndex + 1) { done = true; }
                                }
                                else { ThrowErrorAndLog(Error::UnknownFileNameEncoding, "Unexpected next sequence value"); }
                            }

                            if (!done)
                            {   // We are not done! Point to the next % encoded UTF-8 seq
                                sequenceIndex++;
                                index++;
                            }
                        }
                        else
                        {   // We are looking for a % and we are not done. Abort!
                            ThrowError(Error::UnknownFileNameEncoding);
                        }
                    }
                    ValidateCodepoint(codepoint, sequenceSize);

                    if (codepoint <= 0xFFFF) { result.push_back(codepoint); }
                    else
                    {   // Because of the expected range of codepoints [0x010000, 0x10FFFF], the
                        // subtraction never underflows.  What you end up with is the 11 bits after
                        // the first 10 of the codepoint minus 0x1000 OR'ed with 0xD800.  Since the
                        // max for the codepoint is 0x10FFFF, the max for the 11 bits minus 0x1000
                        // is 0x3FF (and the minimum is 0). OR'ed with D800 gives the range
                        // [0xD800, 0xDBFF] which is the range of the high surrogate.
                        wchar_t ch1 = ((codepoint & 0x00FC00) >> 10) |
                                      (((codepoint & 0x1F0000) - 0x010000) >> 10) |
                                      0x00D800;
                        result.push_back(ch1);
                        // Since the codepoint is AND'ed with 0x3FF (the bottom 10 bits of the
                        // codepoint) and OR'ed with 0xDC00, the possible range is 0xDC00 through
                        // 0xDFFF. This is exactly the range of the low surrogates.
                        wchar_t ch2 = (codepoint & 0x0003FF) | 0x00DC00;
                        result.push_back(ch2);
                    }
                }
            }
            else
            {   result += fileNameW[index];
            }
        }
        return wstring_to_utf8(result);
    }

    // Douglas Crockford's base 32 alphabet variant is 0-9, A-Z except for i, l, o, and u.
    const char base32DigitList[] = "0123456789abcdefghjkmnpqrstvwxyz";

    std::string Base32Encoding(const std::vector<std::uint8_t>& bytes)
    {
        const size_t publisherIdSize = 13;
        const size_t byteCount = 8;

        // Consider groups of five bytes.  This is the smallest number of bytes that has a number of bits
        // that's evenly divisible by five.
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
        char output[publisherIdSize+1] = "";
        size_t outputIndex = 0;
        for(size_t byteIndex = 0; byteIndex < byteCount; byteIndex +=5)
        {
            uint8_t firstByte = bytes[byteIndex];
            uint8_t secondByte = (byteIndex + 1) < byteCount ? bytes[byteIndex + 1] : 0;
            output[outputIndex++] = base32DigitList[(firstByte & 0xF8) >> 3];
            output[outputIndex++] = base32DigitList[((firstByte & 0x07) << 2) | ((secondByte & 0xC0) >> 6)];

            if(byteIndex + 1 < byteCount)
            {
                uint8_t thirdByte = (byteIndex + 2) < byteCount ? bytes[byteIndex + 2] : 0;
                output[outputIndex++] = base32DigitList[(secondByte & 0x3E) >> 1];
                output[outputIndex++] = base32DigitList[((secondByte & 0x01) << 4) | ((thirdByte & 0xF0) >> 4)];

                if(byteIndex + 2 < byteCount)
                {
                    uint8_t fourthByte = (byteIndex + 3) < byteCount ? bytes[byteIndex + 3] : 0;
                    output[outputIndex++] = base32DigitList[((thirdByte & 0x0F) << 1) | ((fourthByte & 0x80) >> 7)];

                    if (byteIndex + 3 < byteCount)
                    {
                        uint8_t fifthByte = (byteIndex + 4) < byteCount ? bytes[byteIndex + 4] : 0;
                        output[outputIndex++] = base32DigitList[(fourthByte & 0x7C) >> 2];
                        output[outputIndex++] = base32DigitList[((fourthByte & 0x03) << 3) | ((fifthByte & 0xE0) >> 5)];

                        if (byteIndex + 4 < byteCount)
                        {
                            output[outputIndex++] = base32DigitList[fifthByte & 0x1F];
                        }
                    }
                }
            }
        }
        output[publisherIdSize] = '\0';
        return std::string(output);
    }

    const std::uint8_t base64DecoderRing[128] =
    {
        /*    0-15 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*   16-31 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*   32-47 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
        /*   48-63 */   52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xFF, 0xFF, 0xFF,   64, 0xFF, 0xFF,
        /*   64-79 */ 0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
        /*   80-95 */   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*  96-111 */ 0xFF,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
        /* 112-127 */   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    std::vector<std::uint8_t> GetBase64DecodedValue(const std::string& value)
    {
        std::vector<std::uint8_t> result;

        ThrowErrorIfNot(Error::InvalidParameter, (0 == (value.length() % 4)), "invalid base64 encoding");
        for(std::size_t index=0; index < value.length(); index += 4)
        {
            ThrowErrorIf(Error::InvalidParameter,(
            (value[index+0] | value[index+1] | value[index+2] | value[index+3]) >= 128
            ), "invalid base64 encoding");

            auto v1 = base64DecoderRing[value[index+0]];
            auto v2 = base64DecoderRing[value[index+1]];
            auto v3 = base64DecoderRing[value[index+2]];
            auto v4 = base64DecoderRing[value[index+3]];

            ThrowErrorIf(Error::InvalidParameter,(((v1 | v2) >= 64) || ((v3 | v4) == 0xFF)), "first two chars of a four char base64 sequence can't be ==, and must be valid");
            ThrowErrorIf(Error::InvalidParameter,(v3 == 64 && v4 != 64), "if the third char is = then the fourth char must be =");
            std::size_t byteCount = (v4 != 64 ? 3 : (v3 != 64 ? 2 : 1));
            result.push_back(static_cast<std::uint8_t>(((v1 << 2) | ((v2 >> 4) & 0x03))));
            if (byteCount >1)
            {
                result.push_back(static_cast<std::uint8_t>(((v2 << 4) | ((v3 >> 2) & 0x0F)) & 0xFF));
                if (byteCount >2)
                {
                    result.push_back(static_cast<std::uint8_t>(((v3 << 6) | ((v4 >> 0) & 0x3F)) & 0xFF));
                }
            }
        }
        return result;
    }

} /*Encoding */ } /* MSIX */
