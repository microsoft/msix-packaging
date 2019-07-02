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

#include <iostream>

using namespace MsixTest::Pack;

constexpr std::uint32_t DefaultBlockSize = 65535;

void InitializePackageWriter(IStream* outputStream, IAppxPackageWriter** packageWriter)
{
    *packageWriter = nullptr;

    MsixTest::ComPtr<IAppxFactory> appxFactory;
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free,
        MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));
    REQUIRE_SUCCEEDED(appxFactory->CreatePackageWriter(outputStream, nullptr, packageWriter));
    return;
}

// Test creating instances of package writer
TEST_CASE("Api_AppxPackageWriter_create", "[api]")
{
    MsixTest::ComPtr<IAppxFactory> appxFactory;
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free,
        MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));

    auto outputStream = MsixTest::Stream("testpackage.msix", false, true);
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
    auto outputStream = MsixTest::Stream("testpackage.msix", false, false);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    // These values are set so that the files added to the package have increasingly
    // larger sizes, with the first file having a small size < DefaultBlockSize, and
    // the last file having a large size > 10x DefaultBlockSize.
    const std::uint32_t contentSizeIncrement = DefaultBlockSize * 10 / static_cast<uint32_t>(TestConstants::GoodFileNames.size()) + 1;
    std::uint32_t contentSize = 10;

    for(const auto& fileName : TestConstants::GoodFileNames)
    {
        // Create file and write random data to it
        auto fileStream = MsixTest::Stream(fileName.first, false, true);
        WriteContentToStream(contentSize, fileStream.Get());
        // TODO, if this fails says with file was it
        CHECK_SUCCEDDED(packageWriter->AddPayloadFile(
            fileName.second.c_str(),
            TestConstants::ContentType,
            APPX_COMPRESSION_OPTION_NORMAL,
            fileStream.Get()));
        contentSize += contentSizeIncrement;
    }

    // Finalize package
    // create manifest stream
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

// Test creating a valid msix package via IAppxPackageWriter.
// Create a package with empty files in start, middle and end positions, 
// and reuse the same content streams packaged under different names.
TEST_CASE("Api_AppxPackageWriter_good_with_empty_files", "[api]")
{
    auto outputStream = MsixTest::Stream("testpackage.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    auto emptyStream = MsixTest::Stream("test_empty_file.txt", false, true);
    auto contentStream = MsixTest::Stream("test_file.txt", false, true);
    WriteContentToStream(618963, contentStream.Get());

    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[0].second.c_str(),
        TestConstants::ContentType,
        APPX_COMPRESSION_OPTION_NORMAL,
        emptyStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[1].second.c_str(),
        TestConstants::ContentType,
        APPX_COMPRESSION_OPTION_NORMAL,
        contentStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[2].second.c_str(),
        TestConstants::ContentType,
        APPX_COMPRESSION_OPTION_NORMAL,
        emptyStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[3].second.c_str(),
        TestConstants::ContentType,
        APPX_COMPRESSION_OPTION_NONE,
        contentStream.Get()));
    REQUIRE_SUCCEEDED(packageWriter->AddPayloadFile(
        TestConstants::GoodFileNames[4].second.c_str(),
        TestConstants::ContentType,
        APPX_COMPRESSION_OPTION_NONE,
        emptyStream.Get()));
    //REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

// Create new package writer to write out a package with no payload files
TEST_CASE("Api_AppxPackageWriter_good_no_payload", "[api]")
{
    auto outputStream = MsixTest::Stream("testpackage.msix", false, true);

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    InitializePackageWriter(outputStream.Get(), &packageWriter);

    //REQUIRE_SUCCEEDED(packageWriter->Close(manifestStream.Get()));

    // Reopen the package, validates that the written package is readable
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(outputStream.Get(), &packageReader);
}

/*
// Test creating a valid msix package via IAppxPackageWriter3
TEST_CASE("Api_AppxPackageWriter_payloadfiles", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}

// Tests failure cases for IAppxPackageWriter
TEST_CASE("Api_AppxPackageWriter_errors", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}
*/