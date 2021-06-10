//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxPackageWriter interface
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "PackTestData.hpp"
#include "macros.hpp"
#include "StreamBase.hpp"

#include <iostream>

using namespace MsixTest::Pack;

constexpr std::uint32_t DefaultBlockSize = 65536;

void InitializePackageWriter(IStream* outputStream, IAppxPackageWriter** packageWriter, bool enableFileHash = false)
{
    *packageWriter = nullptr;

    MsixTest::ComPtr<IAppxFactory> appxFactory;
    if (enableFileHash)
    {
        REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeapAndOptions(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free,
            MSIX_VALIDATION_OPTION_SKIPSIGNATURE, MSIX_FACTORY_OPTION_WRITER_ENABLE_FILE_HASH, &appxFactory));
    }
    else
    {
        REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free,
            MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));
    }
    REQUIRE_SUCCEEDED(appxFactory->CreatePackageWriter(outputStream, nullptr, packageWriter));
    return;
}

void TestAppxPackageWriter_good(LPCSTR outputFileName, bool enableFileHash)
{
    auto outputStream = MsixTest::StreamFile(outputFileName, false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter, enableFileHash);

    // These values are set so that the files added to the package have increasingly
    // larger sizes, with the first file having a small size < DefaultBlockSize, and
    // the last file having a large size > 10x DefaultBlockSize.
    const std::uint32_t contentSizeIncrement = DefaultBlockSize * 10 / static_cast<uint32_t>(TestConstants::GoodFileNames.size()) + 1;
    std::uint32_t contentSize = 10;

    for (const auto& fileName : TestConstants::GoodFileNames)
    {
        // Create file and write random data to it
        auto fileStream = MsixTest::StreamFile(fileName.first, false, true);
        WriteContentToStream(contentSize, fileStream.Get());
        REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
            fileName.second.c_str(),
            TestConstants::ContentType.c_str(),
            APPX_COMPRESSION_OPTION_NORMAL,
            fileStream.Get()));
        contentSize += contentSizeIncrement;
    }

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Test creating instances of package writer
TEST_CASE("Api_AppxPackageWriter_create", "[api]")
{
    MsixTest::ComPtr<IAppxFactory> appxFactory;
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free,
        MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));

    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;

    // Test incorrect nullptr parameter. Note, currently we don't support APPX_PACKAGE_SETTINGS in the MSIX SDK
    // we always create a zip64 package and use SHA256 as hash method, so nullptr as second parameter is fine.
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        appxFactory->CreatePackageWriter(nullptr, nullptr, &packageWriter));
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        appxFactory->CreatePackageWriter(outputStream.Get(), nullptr, nullptr));

    REQUIRE_SUCCEEDED(appxFactory->CreatePackageWriter(outputStream.Get(), nullptr, &packageWriter));
}

// Test creating a valid msix package via IAppxPackageWriter with different file sizes
TEST_CASE("Api_AppxPackageWriter_good", "[api]")
{
    TestAppxPackageWriter_good("test_package.msix", false /* enableFileHash */);
}

// Test creating a valid msix package with file hash enabled in block map via IAppxPackageWriter with different file sizes
TEST_CASE("Api_AppxPackageWriter_FileHashEnabled_good", "[api]")
{
    TestAppxPackageWriter_good("test_package_with_filehash.msix", true /* enableFileHash */);
}

// Test creating a valid msix package via IAppxPackageWriter.
// Create a package with empty files in start, middle and end positions, 
// and reuse the same content streams packaged under different names.
// IAppxPackageWriter::AddPayloadFile calls internally IAppxPackageWriterUtf8::AddPayloadFile
TEST_CASE("Api_AppxPackageWriter_good_with_empty_files", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    auto emptyStream = MsixTest::StreamFile("test_empty_file.txt", false, true);
    auto contentStream = MsixTest::StreamFile("test_file.txt", false, true);
    WriteContentToStream(618963, contentStream.Get());

    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[0].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        emptyStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[1].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        contentStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[2].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        emptyStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[3].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NONE,
        contentStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[4].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NONE,
        emptyStream.Get()));

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Create new package writer to write out a package with no payload files
TEST_CASE("Api_AppxPackageWriter_good_no_payload", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Test creating a valid msix package via IAppxPackageWriter3
TEST_CASE("Api_AppxPackageWriter_payloadfiles", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    std::vector<APPX_PACKAGE_WRITER_PAYLOAD_STREAM> payloadFiles;
    std::vector<MsixTest::StreamFile> streams;
    payloadFiles.resize(TestConstants::GoodFileNames.size());
    streams.resize(TestConstants::GoodFileNames.size());

    // These values are set so that the files added to the package have increasingly
    // larger sizes, with the first file having a small size < DefaultBlockSize, and
    // the last file having a large size > 10x DefaultBlockSize.
    const std::uint32_t contentSizeIncrement = DefaultBlockSize * 10 / static_cast<uint32_t>(TestConstants::GoodFileNames.size()) + 1;
    std::uint32_t contentSize = 10;

    for(size_t i = 0; i < TestConstants::GoodFileNames.size(); i++)
    {
        // Create file and write random data to it
        streams[i].Initialize(TestConstants::GoodFileNames[i].first, false, true);
        WriteContentToStream(contentSize, streams[i].Get());

        payloadFiles[i].fileName = TestConstants::GoodFileNames[i].second.c_str();
        payloadFiles[i].contentType = TestConstants::ContentType.c_str();
        payloadFiles[i].compressionOption = APPX_COMPRESSION_OPTION_NORMAL;
        payloadFiles[i].inputStream = streams[i].Get();
        contentSize += contentSizeIncrement;
    }

    auto packageWriter3 = packageWriter.As<IAppxPackageWriter3>();

    // memoryLimist isn't implemented yet, but set a very small memory limit to force
    // all the handling loops when we do: 320kb.
    REQUIRE_SUCCEEDED(packageWriter3->AddPayloadFiles(
        static_cast<UINT32>(TestConstants::GoodFileNames.size()),
        payloadFiles.data(),
        327680
    ));

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Test creating a valid msix package via IAppxPackageWriter3Utf8
TEST_CASE("Api_AppxPackageWriter_payloadfiles_utf8", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    std::vector<APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8> payloadFiles;
    std::vector<std::string> payloadFilesNames;
    std::vector<MsixTest::StreamFile> streams;
    payloadFiles.resize(TestConstants::GoodFileNames.size());
    streams.resize(TestConstants::GoodFileNames.size());
    payloadFilesNames.resize(TestConstants::GoodFileNames.size());

    std::string contentType = MsixTest::String::utf16_to_utf8(TestConstants::ContentType);

    // These values are set so that the files added to the package have increasingly
    // larger sizes, with the first file having a small size < DefaultBlockSize, and
    // the last file having a large size > 10x DefaultBlockSize.
    const std::uint32_t contentSizeIncrement = DefaultBlockSize * 10 / static_cast<uint32_t>(TestConstants::GoodFileNames.size()) + 1;
    std::uint32_t contentSize = 10;

    for(size_t i = 0; i < TestConstants::GoodFileNames.size(); i++)
    {
        // Create file and write random data to it
        streams[i].Initialize(TestConstants::GoodFileNames[i].first, false, true);
        WriteContentToStream(contentSize, streams[i].Get());

        payloadFilesNames[i] = MsixTest::String::utf16_to_utf8(TestConstants::GoodFileNames[i].second);

        payloadFiles[i].fileName = payloadFilesNames[i].c_str();
        payloadFiles[i].contentType = contentType.c_str();
        payloadFiles[i].compressionOption = APPX_COMPRESSION_OPTION_NORMAL;
        payloadFiles[i].inputStream = streams[i].Get();
        contentSize += contentSizeIncrement;
    }

    auto packageWriter3utf8 = packageWriter.As<IAppxPackageWriter3Utf8>();

    // memoryLimist isn't implemented yet, but set a very small memory limit to force
    // all the handling loops when we do: 320kb.
    REQUIRE_SUCCEEDED(packageWriter3utf8->AddPayloadFiles(
        static_cast<UINT32>(TestConstants::GoodFileNames.size()),
        payloadFiles.data(),
        327680
    ));

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Tests failure cases for IAppxPackageWriter
TEST_CASE("Api_AppxPackageWriter_state_errors", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    auto contentStream = MsixTest::StreamFile("test_file.txt", false, true);
    WriteContentToStream(200, contentStream.Get());

    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);

    // Try to add a file with bad compression option
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        packageWriter->AddPayloadFile(
            TestConstants::GoodFileNames[0].second.c_str(),
            TestConstants::ContentType.c_str(),
            static_cast<APPX_COMPRESSION_OPTION>(12345),
            contentStream.Get()));

    // Package writer should not accept calls after a failure
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidState),
        packageWriter->AddPayloadFile(
            TestConstants::GoodFileNames[0].second.c_str(),
            TestConstants::ContentType.c_str(),
            APPX_COMPRESSION_OPTION_NORMAL,
            contentStream.Get()));
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidState), 
        packageWriter->Close(manifestStream.Get()));
}

TEST_CASE("Api_AppxPackageWriter_invalid_names", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    auto fileStream = MsixTest::StreamFile("test_file.txt", false, true);
    WriteContentToStream(200, fileStream.Get());
    for(const auto& fileName : TestConstants::BadFileNames)
    {
        MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
        InitializePackageWriter(outputStream.Get(), &packageWriter);
        REQUIRE_FAILED(packageWriter->AddPayloadFile(
                fileName.c_str(),
                TestConstants::ContentType.c_str(),
                APPX_COMPRESSION_OPTION_NORMAL,
                fileStream.Get()));
    }
}

TEST_CASE("Api_AppxPackageWriter_closed", "[api]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    auto fileStream = MsixTest::StreamFile("test_file.txt", false, true);
    WriteContentToStream(200, fileStream.Get());

    // Try to add a file after the writer is closed
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidState),
          packageWriter->AddPayloadFile(
            TestConstants::GoodFileNames[0].second.c_str(),
            TestConstants::ContentType.c_str(),
            APPX_COMPRESSION_OPTION_NORMAL,
            fileStream.Get()));
}

TEST_CASE("Api_AppxPackageWriter_add_same_payload_file")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    auto contentStream = MsixTest::StreamFile("test_file.txt", false, true);
    WriteContentToStream(618963, contentStream.Get());

    // Adding file same file twice
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[1].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        contentStream.Get()));

    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::DuplicateFile),
        packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[1].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        contentStream.Get()));

    // The package should be in an invalid state now.
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidState),
        packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[0].second.c_str(),
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        contentStream.Get()));
}

class GeneratedEasilyCompressedFileStream final : public MSIX::StreamBase
{
public:
    GeneratedEasilyCompressedFileStream(uint64_t size) : m_size(size) {}

    // IStream
    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* newPosition) noexcept override try
    {
        // Determine new range relative position
        LARGE_INTEGER newPos = { 0 };
        switch (origin)
        {
        case Reference::CURRENT:
            newPos.QuadPart = m_offset + move.QuadPart;
            break;
        case Reference::START:
            newPos.QuadPart = move.QuadPart;
            break;
        case Reference::END:
            newPos.QuadPart = m_size + move.QuadPart;
            break;
        }

        // Constrain newPos to range relative values
        if (newPos.QuadPart < 0)
        {
            m_offset = 0;
        }
        else
        {
            m_offset = std::min(static_cast<uint64_t>(newPos.QuadPart), m_size);
        }

        if (newPosition) { newPosition->QuadPart = m_offset; }
        return static_cast<HRESULT>(MSIX::Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
    {
        uint64_t bytesToRead = std::min(m_size - m_offset, static_cast<uint64_t>(countBytes));
        // We can't really fail so just put the value in directly.
        if (bytesRead) { *bytesRead = static_cast<ULONG>(bytesToRead); }

        while (bytesToRead)
        {
            uint64_t block = m_offset / DefaultBlockSize;
            uint64_t endOfBlock = (block + 1) * DefaultBlockSize;
            uint64_t bytesToWrite = std::min(endOfBlock, m_size) - m_offset;
            bytesToWrite = std::min(bytesToWrite, bytesToRead);
            memset(buffer, static_cast<int>(block % 256), static_cast<size_t>(bytesToWrite));

            buffer = static_cast<void*>(static_cast<int8_t*>(buffer) + bytesToWrite);
            m_offset += bytesToWrite;
            bytesToRead -= bytesToWrite;
        }

        return static_cast<HRESULT>(MSIX::Error::OK);
    } CATCH_RETURN();

protected:
    uint64_t m_size = 0;
    uint64_t m_offset = 0;
};

// Test creating a valid msix package with a contained file that is larger than 4GB
// The package itself will be much smaller; do not unpack the package from this test
TEST_CASE("Api_AppxPackageWriter_file_over_4GB", "[api][.slow]")
{
    auto outputStream = MsixTest::StreamFile("test_package.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    // Create stream to generate our very compressable data with more than 4GB of data
    auto fileStream = MsixTest::ComPtr<IStream>::Make<GeneratedEasilyCompressedFileStream>(0x100000100);
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        L"largefile.bin",
        TestConstants::ContentType.c_str(),
        APPX_COMPRESSION_OPTION_NORMAL,
        fileStream.Get()));

    // Finalize package, create manifest stream
    MsixTest::ComPtr<IStream> manifestStream;
    MakeManifestStream(&manifestStream);
    REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    // return to the beginning
    LARGE_INTEGER zero = { 0 };
    REQUIRE_SUCCEEDED(outputStream.Get()->Seek(zero, STREAM_SEEK_SET, nullptr));
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}
