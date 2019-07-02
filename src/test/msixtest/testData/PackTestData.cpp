//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "PackTestData.hpp"
#include "msixtest_int.hpp"
#include "macros.hpp"

#include <array>
#include <algorithm>

namespace MsixTest { namespace Pack {

    // Randomly generated data to be used as filler content in the packages being produced.
    // The length is chosen so that when the sequence is repeatedly written to a large stream
    // then divided into blocks of size 65536, the resulting blocks will begin at different
    // offsets of the sequence and therefore have different data/hashes.
    const std::array<std::uint8_t, 439> RandomData = {
        20, 209, 116, 213, 153, 242, 196, 56, 42, 220, 215, 200, 35, 40, 124,
        117, 200, 151, 213, 79, 98, 128, 168, 217, 138, 13, 83, 231, 32, 128,
        185, 18, 214, 11, 90, 242, 7, 207, 211, 2, 181, 115, 13, 42, 22, 210,
        185, 207, 208, 75, 62, 198, 87, 11, 170, 127, 158, 123, 119, 61, 253,
        16, 230, 134, 138, 112, 221, 194, 7, 250, 13, 250, 131, 235, 133, 117,
        183, 141, 116, 146, 245, 44, 233, 1, 182, 191, 130, 198, 180, 145, 239,
        218, 57, 0, 19, 37, 85, 216, 173, 11, 249, 129, 17, 255, 85, 208, 243,
        150, 85, 106, 39, 181, 152, 248, 156, 151, 93, 235, 93, 118, 9, 178,
        47, 76, 10, 52, 207, 2, 108, 203, 144, 186, 162, 109, 139, 172, 63, 32,
        157, 52, 116, 71, 110, 11, 99, 109, 196, 10, 35, 232, 205, 245, 40, 126,
        192, 141, 144, 3, 99, 60, 18, 88, 16, 42, 84, 110, 86, 116, 193, 145,
        228, 245, 134, 222, 145, 181, 76, 156, 138, 201, 234, 65, 41, 1, 209,
        115, 254, 95, 220, 178, 76, 26, 191, 221, 209, 46, 122, 172, 42, 252,
        90, 7, 88, 69, 30, 220, 121, 163, 76, 232, 122, 14, 137, 126, 4, 173,
        140, 248, 167, 137, 193, 76, 87, 161, 28, 141, 99, 113, 134, 74, 229,
        38, 105, 157, 28, 189, 238, 202, 115, 93, 189, 188, 7, 252, 123, 33,
        231, 180, 11, 204, 148, 17, 247, 117, 120, 234, 187, 19, 174, 46, 1,
        166, 196, 138, 144, 62, 119, 199, 4, 233, 69, 243, 48, 144, 254, 96,
        42, 164, 141, 200, 173, 157, 5, 167, 7, 162, 242, 77, 120, 183, 110,
        255, 1, 48, 100, 103, 15, 72, 225, 74, 103, 197, 123, 21, 64, 120, 164,
        207, 196, 96, 215, 247, 13, 141, 115, 98, 14, 117, 162, 61, 192, 82,
        207, 212, 113, 36, 52, 216, 98, 73, 70, 11, 228, 83, 203, 21, 164, 93,
        39, 94, 130, 166, 142, 253, 133, 125, 252, 79, 245, 74, 176, 33, 58,
        233, 232, 103, 218, 210, 55, 132, 160, 239, 204, 173, 125, 87, 66, 185,
        45, 195, 207, 67, 143, 107, 49, 108, 230, 25, 239, 222, 245, 200, 8,
        90, 65, 120, 188, 221, 78, 247, 42, 223, 134, 235, 165, 56, 145, 171,
        168, 55, 165, 206, 53, 86, 240, 210, 47, 146, 17, 197, 70, 237, 111,
        43, 92, 30, 51, 108, 144, 8, 207, 96, 195, 216, 173, 29, 106, 115, 226,
        217, 73, 175, 11, 121, 74, 93, 10, 197, 238
    };
    std::uint64_t randomDataOffset = 0;

    void WriteContentToStream(std::uint64_t streamSize, IStream* contentStream)
    {
        // Write repeated sequences of RandomData to the stream
        std::uint64_t sizeRemainingToWrite = streamSize;
        while (sizeRemainingToWrite > 0)
        {
            if (randomDataOffset >= RandomData.size())
            {
                randomDataOffset = 0;
            }
            std::uint64_t randomDataLengthAfterOffset = RandomData.size() - randomDataOffset;
            UINT32 sizeToWrite = static_cast<UINT32>(std::min(sizeRemainingToWrite, randomDataLengthAfterOffset));
            REQUIRE_SUCCEEDED(contentStream->Write(RandomData.data() + randomDataOffset, sizeToWrite, nullptr));
            sizeRemainingToWrite -= sizeToWrite;
            randomDataOffset += sizeToWrite;
        }
        // return to the beginning
        LARGE_INTEGER zero = { 0 };
        REQUIRE_SUCCEEDED(contentStream->Seek(zero, STREAM_SEEK_SET, nullptr));
    }

    
    void MakeManifestStream(IStream** manifestStream)
    {
        auto manifestPath = TestPath::GetInstance()->GetPath(TestPath::Directory::Pack) + "/" + "Manifest_Good.xml";
        auto stream = Stream(manifestPath, true);
        *manifestStream = stream.Detach(); 
    }


} }
