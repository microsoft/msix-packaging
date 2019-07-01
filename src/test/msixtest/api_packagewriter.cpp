//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxPackageWriter interface
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "macros.hpp"

void InitializePackageWriter(IStream* outputStream, IAppxPackageWriter** packageWriter)
{
    *packageWriter = nullptr;

    MsixTest::ComPtr<IAppxFactory> appxFactory;
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(Allocators::Allocate, Allocators::Free, MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));
    REQUIRE_SUCCEEDED(appxFactory->CreatePackageWriter(outputStream, nullptr, packageWriter));
    return;
}

// Test creating instances of package writer
TEST_CASE("Api_AppxPackageWriter_create", "[api]")
{
    MsixTest::ComPtr<IAppxFactory> appxFactory;
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(Allocators::Allocate, Allocators::Free, MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &appxFactory));

    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix", false, true);

    // Test incorrect nullptr parameter. Note, currently we don't support APPX_PACKAGE_SETTINGS in the MSIX SDK
    // we always create a zip64 package and use SHA256 as hash method, so nullptr as second parameter is fine.
    REQUIRE_HR((static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        appxFactory->CreatePackageWriter(nullptr, nullptr, packageWriter));
    REQUIRE_HR((static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        appxFactory->CreatePackageWriter(outputStream.Get(), nullptr, nullptr));

    REQUIRE_SUCCEEDED(appxFactory->CreatePackageWriter(outputStream.Get(), nullptr, packageWriter));
}

// Test creating a valid msix package via IAppxPackageWriter
TEST_CASE("Api_AppxPackageWriter_payloadfile", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}

// Test creating a valid msix package via IAppxPackageWriterUtf8
TEST_CASE("Api_AppxPackageWriter_payloadfile_utf8", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}

// Test creating a valid msix package via IAppxPackageWriter3
TEST_CASE("Api_AppxPackageWriter_payloadfiles", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}

// Test creating a valid msix package via IAppxPackageWriterUtf8
TEST_CASE("Api_AppxPackageWriter_payloadfile_utf8", "[api]")
{
    MsixTest::ComPtr<IAppxPackageWriter> packageWriter;
    auto outputStream = MsixTest::Stream("pack/package.msix");
    InitializePackageWriter(outputStream.Get(), &packageWriter);
}

// Test creating a valid msix package via IAppxPackageWriter3Utf8
TEST_CASE("Api_AppxPackageWriter_payloadfiles_utf8", "[api]")
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
