//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxPackageReader interface
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "UnpackTestData.hpp"
#include "BlockMapTestData.hpp"
#include "macros.hpp"

#include <iostream>
#include <array>

// Validates all payload files from the package are correct
TEST_CASE("Api_AppxPackageReader_PayloadFiles", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);

    // Expected files in the package
    auto expectedFiles = MsixTest::Unpack::GetExpectedFiles();

    MsixTest::ComPtr<IAppxFilesEnumerator> files;
    REQUIRE_SUCCEEDED(packageReader->GetPayloadFiles(&files));
    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(files->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxFile> file;
        REQUIRE_SUCCEEDED(files->GetCurrent(&file));

        MsixTest::Wrappers::Buffer<wchar_t> fileName;
        REQUIRE_SUCCEEDED(file->GetName(&fileName));
        auto name = fileName.ToString();
        std::replace(name.begin(), name.end(), '\\', '/'); // expected files uses / separator
        auto find = expectedFiles.find(name);
        REQUIRE(find != expectedFiles.end());
        expectedFiles.erase(find);

        MsixTest::ComPtr<IAppxFileUtf8> fileUtf8;
        REQUIRE_SUCCEEDED(file->QueryInterface(UuidOfImpl<IAppxFileUtf8>::iid, reinterpret_cast<void**>(&fileUtf8)));
        MsixTest::Wrappers::Buffer<char> fileNameUtf8;
        REQUIRE_SUCCEEDED(fileUtf8->GetName(&fileNameUtf8));
        REQUIRE(fileName.ToString() == fileNameUtf8.ToString());

        // Compare that the file from GetPayloadFile is the same file
        MsixTest::ComPtr<IAppxFile> file2;
        REQUIRE_SUCCEEDED(packageReader->GetPayloadFile(fileName.Get(), &file2));
        REQUIRE_ARE_SAME(file.Get(), file2.Get());

        MsixTest::ComPtr<IAppxPackageReaderUtf8> packageReaderUtf8;
        REQUIRE_SUCCEEDED(packageReader->QueryInterface(UuidOfImpl<IAppxPackageReaderUtf8>::iid, reinterpret_cast<void**>(&packageReaderUtf8)));
        MsixTest::ComPtr<IAppxFile> file3;
        REQUIRE_SUCCEEDED(packageReaderUtf8->GetPayloadFile(fileNameUtf8.Get(), &file3));
        REQUIRE_ARE_SAME(file2.Get(), file3.Get());

        REQUIRE_SUCCEEDED(files->MoveNext(&hasCurrent));
    }
    // The only files left should be footprint files
    expectedFiles.erase(MsixTest::Constants::Package::AppxBlockMap.second);
    expectedFiles.erase(MsixTest::Constants::Package::AppxManifest.second);
    expectedFiles.erase(MsixTest::Constants::Package::AppxSignature.second);
    expectedFiles.erase(MsixTest::Constants::Package::CodeIntegrity.second);
    REQUIRE(expectedFiles.empty());
}

// Verifies a payload file information from the package
TEST_CASE("Api_AppxPackageReader_PayloadFile", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxFile;

    // Even on non windows, GetPayloadFile expects a '\'
    std::string fileName = "Assets\\video_offline_demo_page2.jpg";
    auto fileNameW = MsixTest::String::utf8_to_utf16(fileName);

    REQUIRE_SUCCEEDED(packageReader->GetPayloadFile(fileNameW.c_str(), &appxFile));
    REQUIRE_NOT_NULL(appxFile.Get());

    MsixTest::Wrappers::Buffer<wchar_t> appxFileName;
    REQUIRE_SUCCEEDED(appxFile->GetName(&appxFileName));
    REQUIRE(fileName == appxFileName.ToString());

    APPX_COMPRESSION_OPTION fileCompression;
    REQUIRE_SUCCEEDED(appxFile->GetCompressionOption(&fileCompression));
    REQUIRE(APPX_COMPRESSION_OPTION_NONE == fileCompression);

    UINT64 fileSize;
    REQUIRE_SUCCEEDED(appxFile->GetSize(&fileSize));
    REQUIRE(78720 == static_cast<std::uint64_t>(fileSize));
}

// Validate a file is not in the package.
TEST_CASE("Api_AppxPackageReader_PayloadFile_DoesNotExist", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxFile;
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::FileNotFound),
        packageReader->GetPayloadFile(L"thisIsAFakeFile.txt", &appxFile));
}

// Validates a footprint files
TEST_CASE("Api_AppxPackageReader_FootprintFile", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    MsixTest::InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxBlockMap;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxBlockMap.first, &appxBlockMap));
    MsixTest::Wrappers::Buffer<wchar_t> appxBlockMapName;
    REQUIRE_SUCCEEDED(appxBlockMap->GetName(&appxBlockMapName));
    REQUIRE(MsixTest::Constants::Package::AppxBlockMap.second == appxBlockMapName.ToString());

    MsixTest::ComPtr<IAppxFile> appxManifest;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxManifest.first, &appxManifest));
    MsixTest::Wrappers::Buffer<wchar_t> appxManifestName;
    REQUIRE_SUCCEEDED(appxManifest->GetName(&appxManifestName));
    REQUIRE(MsixTest::Constants::Package::AppxManifest.second == appxManifestName.ToString());

    MsixTest::ComPtr<IAppxFile> appxSignature;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxSignature.first, &appxSignature));
    MsixTest::Wrappers::Buffer<wchar_t> appxSignatureName;
    REQUIRE_SUCCEEDED(appxSignature->GetName(&appxSignatureName));
    REQUIRE(MsixTest::Constants::Package::AppxSignature.second == appxSignatureName.ToString());

    MsixTest::ComPtr<IAppxFile> appxCodeIntegrity;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::CodeIntegrity.first, &appxCodeIntegrity));
    MsixTest::Wrappers::Buffer<wchar_t> appxCodeIntegrityName;
    REQUIRE_SUCCEEDED(appxCodeIntegrity->GetName(&appxCodeIntegrityName));
    auto codeIntegrityName = MsixTest::Constants::Package::CodeIntegrity.second;
    std::replace(codeIntegrityName.begin(), codeIntegrityName.end(), '/', '\\');
    REQUIRE(codeIntegrityName == appxCodeIntegrityName.ToString());
}
