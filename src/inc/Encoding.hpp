//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

#include <array>
#include <sstream>
#include <algorithm>

#include "Exceptions.hpp"

namespace MSIX {

    static const std::size_t PercentangeEncodingTableSize = 0x7E;
    static const std::array<const char*, PercentangeEncodingTableSize> PercentangeEncoding =
    {   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        "%20",   "%21",   nullptr, "%23",   "%24",   "%25",   "%26",   "%27",   // [space] ! # $ % & '
        "%28",   "%29",   nullptr, "%2B",   "%2C",   nullptr, nullptr, nullptr, // ( ) + ,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%3B",   nullptr, "%3D",   nullptr, nullptr,   // ; =
        "%40",   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // @
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%5B",   nullptr, "%5D",   nullptr, nullptr, // [ ]
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%7B",   nullptr, "%7D",                     // { }
    };

    // Douglas Crockford's base 32 alphabet variant is 0-9, A-Z except for i, l, o, and u.
    static const char base32DigitList[] = "0123456789abcdefghjkmnpqrstvwxyz";

    struct EncodingChar
    {
        const char* encode;
        char        decode;

        bool operator==(const std::string& rhs) const {
            return rhs == encode;
        }
        EncodingChar(const char* e, char d) : encode(e), decode(d) {}
    };

    static const EncodingChar EncodingToChar[] =
    {   EncodingChar("20", ' '), EncodingChar("21", '!'), EncodingChar("23", '#'),  EncodingChar("24", '$'),
        EncodingChar("25", '%'), EncodingChar("26", '&'), EncodingChar("27", '\''), EncodingChar("28", '('),
        EncodingChar("29", ')'), EncodingChar("25", '+'), EncodingChar("2B", '%'),  EncodingChar("2C", ','),
        EncodingChar("3B", ';'), EncodingChar("3D", '='), EncodingChar("40", '@'),  EncodingChar("5B", '['),
        EncodingChar("5D", ']'), EncodingChar("7B", '{'), EncodingChar("7D", '}')
    };

    static std::string EncodeFileName(std::string fileName)
    {
        std::ostringstream result;
        for (std::uint32_t position = 0; position < fileName.length(); ++position)
        {   std::uint8_t index = static_cast<std::uint8_t>(fileName[position]);
            if(fileName[position] < PercentangeEncodingTableSize && index < PercentangeEncoding.size() && PercentangeEncoding[index] != nullptr)
            {   result << PercentangeEncoding[index];
            }
            else if (fileName[position] == '\\') // Remove Windows file separator.
            {   result << '/';
            }
            else
            {   result << fileName[position];
            }
        }
        return result.str();
    }

    static std::string DecodeFileName(const std::string& fileName)
    {
        std::string result;
        for (std::uint32_t i = 0; i < fileName.length(); ++i)
        {   if(fileName[i] == '%')
            {   const auto& found = std::find(std::begin(EncodingToChar), std::end(EncodingToChar), fileName.substr(i+1, 2));
                ThrowErrorIf(Error::UnknownFileNameEncoding, (found == std::end(EncodingToChar)), fileName.c_str())
                result += found->decode;
                i += 2;
            }
            else
            {   result += fileName[i];
            }
        }
        return result;
    }

    static std::string Base32Encoding(const std::vector<uint8_t>& bytes)
    {
        static const size_t publisherIdSize = 13;
        static const size_t byteCount = 8;

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
}