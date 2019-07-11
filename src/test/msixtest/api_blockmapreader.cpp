//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxBlockMapReader interface
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "BlockMapTestData.hpp"
#include "macros.hpp"

#include <iostream>
#include <array>

// Validates IAppxBlockMapReader::GetStream
TEST_CASE("Api_AppxBlockMapReader_Stream", "[api]")
{
    std::string package = "TestAppxPackage_Win32.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxBlockMapReader> blockMapReader;
    REQUIRE_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    REQUIRE_NOT_NULL(blockMapReader.Get());

    MsixTest::ComPtr<IStream> stream;
    REQUIRE_SUCCEEDED(blockMapReader->GetStream(&stream));
    REQUIRE_NOT_NULL(stream.Get());
}

// Validates all files information in the blockmap, IAppxBlockMapReaderUtf8, 
// IAppxBlockMapFilesEnumerator, IAppxBlockMapFile and IAppxBlockMapBlock
TEST_CASE("Api_AppxBlockMapReader_Files", "[api]")
{
    std::string package = "TestAppxPackage_Win32.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxBlockMapReader> blockMapReader;
    REQUIRE_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    REQUIRE_NOT_NULL(blockMapReader.Get());

    auto expectedBlockMapFiles = MsixTest::BlockMap::GetExpectedBlockMapFiles();

    MsixTest::ComPtr<IAppxBlockMapReaderUtf8> blockMapReaderUtf8;
    REQUIRE_SUCCEEDED(blockMapReader->QueryInterface(UuidOfImpl<IAppxBlockMapReaderUtf8>::iid, reinterpret_cast<void**>(&blockMapReaderUtf8)));

    MsixTest::ComPtr<IAppxBlockMapFilesEnumerator> blockMapFiles;
    REQUIRE_SUCCEEDED(blockMapReader->GetFiles(&blockMapFiles));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(blockMapFiles->GetHasCurrent(&hasCurrent));
    size_t numOfBlockMapFiles = 0;
    while(hasCurrent)
    {
        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile;
        REQUIRE_SUCCEEDED(blockMapFiles->GetCurrent(&blockMapFile));

        auto expectedFile = expectedBlockMapFiles.at(numOfBlockMapFiles);

        auto compareBlock = [](const std::vector<std::uint8_t>& expected, BYTE* buffer) -> bool
        {
            for(int i = 0; i < expected.size(); ++i)
            {
                if (expected[i] != static_cast<std::uint8_t>(buffer[i]))
                {
                    return false;
                }
            }
            return true;
        };

        MsixTest::Wrappers::Buffer<wchar_t> fileName;
        REQUIRE_SUCCEEDED(blockMapFile->GetName(&fileName));
        REQUIRE(expectedFile.name == fileName.ToString());

        MsixTest::ComPtr<IAppxBlockMapFileUtf8> blockMapFileUtf8;
        REQUIRE_SUCCEEDED(blockMapFile->QueryInterface(UuidOfImpl<IAppxBlockMapFileUtf8>::iid, reinterpret_cast<void**>(&blockMapFileUtf8)));
        MsixTest::Wrappers::Buffer<char> fileNameUtf8;
        REQUIRE_SUCCEEDED(blockMapFileUtf8->GetName(&fileNameUtf8));
        REQUIRE(expectedFile.name == fileNameUtf8.ToString());

        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile2;
        REQUIRE_SUCCEEDED(blockMapReader->GetFile(fileName.Get(), &blockMapFile2));
        REQUIRE_ARE_SAME(blockMapFile.Get(), blockMapFile2.Get());

        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile3;
        REQUIRE_SUCCEEDED(blockMapReaderUtf8->GetFile(fileNameUtf8.Get(), &blockMapFile3));
        REQUIRE_ARE_SAME(blockMapFile.Get(), blockMapFile3.Get());

        UINT32 lfh = 0;
        REQUIRE_SUCCEEDED(blockMapFile->GetLocalFileHeaderSize(&lfh));
        REQUIRE(expectedFile.lfh == static_cast<std::uint32_t>(lfh));

        UINT64 size = 0;
        REQUIRE_SUCCEEDED(blockMapFile->GetUncompressedSize(&size));
        REQUIRE(expectedFile.size == size);

        // Compare blocks
        MsixTest::ComPtr<IAppxBlockMapBlocksEnumerator> blockEnum;
        REQUIRE_SUCCEEDED(blockMapFile->GetBlocks(&blockEnum));
        BOOL hasCurrentBlock = FALSE;
        REQUIRE_SUCCEEDED(blockEnum->GetHasCurrent(&hasCurrentBlock));
        size_t numOfBlocks = 0;
        while (hasCurrentBlock)
        {
            MsixTest::ComPtr<IAppxBlockMapBlock> block;
            REQUIRE_SUCCEEDED(blockEnum->GetCurrent(&block));

            auto expectedBlock = expectedFile.blocks[numOfBlocks];

            UINT32 bufferSize = 0;
            MsixTest::Wrappers::Buffer<BYTE> buffer;
            REQUIRE_SUCCEEDED(block->GetHash(&bufferSize, &buffer));
            REQUIRE(expectedBlock.hash.size() == bufferSize);
            REQUIRE(compareBlock(expectedBlock.hash, buffer.Get()));

            UINT32 compSize = 0;
            REQUIRE_SUCCEEDED(block->GetCompressedSize(&compSize));
            REQUIRE(expectedBlock.compressedSize == static_cast<std::uint32_t>(compSize));

            REQUIRE_SUCCEEDED(blockEnum->MoveNext(&hasCurrentBlock));
            numOfBlocks++;
        }
        REQUIRE(expectedFile.blocks.size() == numOfBlocks);

        REQUIRE_SUCCEEDED(blockMapFiles->MoveNext(&hasCurrent));
        numOfBlockMapFiles++;
    }
    REQUIRE(expectedBlockMapFiles.size() == numOfBlockMapFiles);
}
