//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// This test is for the public APIs of the SDK with already defined expected results.
#include "ApiTest.hpp"

HRESULT TestBlockMapReader(IAppxBlockMapReader* blockMapReader)
{
    ComPtr<IAppxBlockMapFilesEnumerator> blockMapFiles;
    VERIFY_SUCCEEDED(blockMapReader->GetFiles(&blockMapFiles));
    BOOL hasCurrent = FALSE;
    int nBlockFiles = 0;
    VERIFY_SUCCEEDED(blockMapFiles->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxBlockMapFile> blockMapFile;
        VERIFY_SUCCEEDED(blockMapFiles->GetCurrent(&blockMapFile));
        Text fileName;
        VERIFY_SUCCEEDED(blockMapFile->GetName(&fileName));
        VERIFY_WSTR(fileName.Get(), expectedBlockMapFileList[nBlockFiles].name);

        UINT32 lfh;
        VERIFY_SUCCEEDED(blockMapFile->GetLocalFileHeaderSize(&lfh));
        VERIFY_ARE_EQUAL(lfh, expectedBlockMapFileList[nBlockFiles].lfh);

        UINT64 size;
        VERIFY_SUCCEEDED(blockMapFile->GetUncompressedSize(&size));
        VERIFY_ARE_EQUAL(size, expectedBlockMapFileList[nBlockFiles].size);

        ComPtr<IAppxBlockMapBlocksEnumerator> blockEnum;
        VERIFY_SUCCEEDED(blockMapFile->GetBlocks(&blockEnum));
        BOOL hasCurrentBlock = FALSE;
        VERIFY_SUCCEEDED(blockEnum->GetHasCurrent(&hasCurrentBlock));
        int nBlocks = 0;
        while (hasCurrentBlock)
        {
            ComPtr<IAppxBlockMapBlock> block;
            VERIFY_SUCCEEDED(blockEnum->GetCurrent(&block));
            UINT32 bufferSize = 0;
            BYTE* buffer = nullptr;
            VERIFY_SUCCEEDED(block->GetHash(&bufferSize, &buffer));
            VERIFY_ARE_EQUAL(bufferSize, expectedBlockMapFileList[nBlockFiles].blocks[nBlocks].hash.size());
            for (UINT32 i = 0; i < bufferSize; i++)
            {
                VERIFY_ARE_EQUAL(buffer[i], expectedBlockMapFileList[nBlockFiles].blocks[nBlocks].hash[i]);
            }

            UINT32 compsize;
            VERIFY_SUCCEEDED(block->GetCompressedSize(&compsize));
            VERIFY_ARE_EQUAL(compsize, expectedBlockMapFileList[nBlockFiles].blocks[nBlocks].compressedSize);

            VERIFY_SUCCEEDED(blockEnum->MoveNext(&hasCurrentBlock));
            nBlocks++;
        }

        ComPtr<IAppxBlockMapFile> blockMapFile2;
        VERIFY_SUCCEEDED(blockMapReader->GetFile(fileName.Get(), &blockMapFile2));
        VERIFY_ARE_SAME(blockMapFile.Get(), blockMapFile2.Get());

        VERIFY_SUCCEEDED(blockMapFiles->MoveNext(&hasCurrent));
        nBlockFiles++;
    }
    VERIFY_ARE_EQUAL(expectedBlockMapFileList.size(), nBlockFiles)

    ComPtr<IStream> stream;
    VERIFY_SUCCEEDED(blockMapReader->GetStream(&stream));
    VERIFY_NOT_NULL(stream.Get());

    return S_OK;
}

void TestAppxManifestPackageId(IAppxManifestPackageId* packageId)
{
    Text name;
    VERIFY_SUCCEEDED(packageId->GetName(&name));
    VERIFY_WSTR(name.Get(), L"20477fca-282d-49fb-b03e-371dca074f0f");

    APPX_PACKAGE_ARCHITECTURE architecture;
    VERIFY_SUCCEEDED(packageId->GetArchitecture(&architecture));
    VERIFY_ARE_EQUAL(architecture, APPX_PACKAGE_ARCHITECTURE_X86);

    Text publisher;
    VERIFY_SUCCEEDED(packageId->GetPublisher(&publisher));
    VERIFY_WSTR(publisher.Get(), L"CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US");

    BOOL isSame = FALSE;
    VERIFY_SUCCEEDED(packageId->ComparePublisher(publisher.Get(), &isSame));
    VERIFY_IS_TRUE(isSame);

    VERIFY_SUCCEEDED(packageId->ComparePublisher(L"OtherPublisher", &isSame));
    VERIFY_IS_TRUE(!isSame);

    UINT64 packageVersion;
    VERIFY_SUCCEEDED(packageId->GetVersion(&packageVersion));
    VERIFY_ARE_EQUAL(packageVersion, 281474976710656);

    Text resourceId;
    VERIFY_SUCCEEDED(packageId->GetResourceId(&resourceId));
    VERIFY_IS_NULL(resourceId.Get());

    Text packageFullName;
    VERIFY_SUCCEEDED(packageId->GetPackageFullName(&packageFullName));
    VERIFY_WSTR(packageFullName.Get(), L"20477fca-282d-49fb-b03e-371dca074f0f_1.0.0.0_x86__8wekyb3d8bbwe");

    Text packageFamilyName;
    VERIFY_SUCCEEDED(packageId->GetPackageFamilyName(&packageFamilyName));
    VERIFY_WSTR(packageFamilyName.Get(), L"20477fca-282d-49fb-b03e-371dca074f0f_8wekyb3d8bbwe");

    return;
}

void TestAppxManifestReader(IAppxManifestReader* manifestReader)
{
    ComPtr<IStream> stream;
    VERIFY_SUCCEEDED(manifestReader->GetStream(&stream));
    VERIFY_NOT_NULL(stream.Get());

    ComPtr<IAppxManifestApplicationsEnumerator> enumerator;
    VERIFY_SUCCEEDED(manifestReader->GetApplications(&enumerator));
    BOOL hasCurrent = FALSE;
    VERIFY_SUCCEEDED(enumerator->GetHasCurrent(&hasCurrent));
    int nAumids = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxManifestApplication> app;
        VERIFY_SUCCEEDED(enumerator->GetCurrent(&app));
        Text aumid;
        VERIFY_SUCCEEDED(app->GetAppUserModelId(&aumid));
        VERIFY_WSTR(aumid.Get(), L"20477fca-282d-49fb-b03e-371dca074f0f_8wekyb3d8bbwe!App");
        VERIFY_SUCCEEDED(enumerator->MoveNext(&hasCurrent));
        nAumids++;
    }
    VERIFY_ARE_EQUAL(nAumids, 1);

    ComPtr<IAppxManifestProperties> properties;
    VERIFY_SUCCEEDED(manifestReader->GetProperties(&properties));
    BOOL value = TRUE;
    VERIFY_SUCCEEDED(properties->GetBoolValue(L"Framework", &value));
    VERIFY_IS_TRUE(value == FALSE);
    value = FALSE;
    VERIFY_SUCCEEDED(properties->GetBoolValue(L"ResourcePackage", &value));
    VERIFY_IS_TRUE(!value);
    value = FALSE;
    VERIFY_SUCCEEDED(properties->GetBoolValue(L"AllowExecution", &value));
    VERIFY_IS_TRUE(value);
    VERIFY_HR(E_INVALIDARG, properties->GetBoolValue(L"NotValid", &value))

    Text displayName;
    VERIFY_SUCCEEDED(properties->GetStringValue(L"DisplayName", &displayName));
    VERIFY_WSTR(displayName.Get(), L"ms-resource:appName");
    Text publisherDisplayName;
    VERIFY_SUCCEEDED(properties->GetStringValue(L"PublisherDisplayName", &publisherDisplayName));
    VERIFY_WSTR(publisherDisplayName.Get(), L"Microsoft Corporation");
    Text logo;
    VERIFY_SUCCEEDED(properties->GetStringValue(L"Logo", &logo));
    VERIFY_WSTR(logo.Get(), L"Assets\\StoreLogo.png");
    Text description;
    VERIFY_SUCCEEDED(properties->GetStringValue(L"Description", &description));
    VERIFY_IS_NULL(description.Get());
    VERIFY_HR(E_INVALIDARG, properties->GetStringValue(L"NotValid", &description))

    ComPtr<IAppxManifestPackageDependenciesEnumerator> dependencies;
    VERIFY_SUCCEEDED(manifestReader->GetPackageDependencies(&dependencies));
    hasCurrent = FALSE;
    VERIFY_SUCCEEDED(dependencies->GetHasCurrent(&hasCurrent));
    int nDependencies = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxManifestPackageDependency> dependency;
        VERIFY_SUCCEEDED(dependencies->GetCurrent(&dependency));
        Text name;
        VERIFY_SUCCEEDED(dependency->GetName(&name));
        VERIFY_WSTR(name.Get(), L"Microsoft.VCLibs.140.00");
        UINT64 min = 0;
        VERIFY_SUCCEEDED(dependency->GetMinVersion(&min));
        VERIFY_ARE_EQUAL(min, 3940651254874112);
        Text publisher;
        VERIFY_SUCCEEDED(dependency->GetPublisher(&publisher));
        VERIFY_WSTR(publisher.Get(), L"CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US");
        VERIFY_SUCCEEDED(dependencies->MoveNext(&hasCurrent));
        nDependencies++;
    }
    VERIFY_ARE_EQUAL(nDependencies, 1);

    APPX_CAPABILITIES capabilities;
    VERIFY_SUCCEEDED(manifestReader->GetCapabilities(&capabilities));
    VERIFY_ARE_EQUAL(capabilities, APPX_CAPABILITIES::APPX_CAPABILITY_INTERNET_CLIENT);

    ComPtr<IAppxManifestResourcesEnumerator> resources;
    VERIFY_SUCCEEDED(manifestReader->GetResources(&resources));
    hasCurrent = FALSE;
    VERIFY_SUCCEEDED(resources->GetHasCurrent(&hasCurrent));
    int nRes = 0;
    while (hasCurrent)
    {
        Text resource;
        VERIFY_SUCCEEDED(resources->GetCurrent(&resource));
        VERIFY_WSTR(resource.Get(), L"EN-US");
        VERIFY_SUCCEEDED(resources->MoveNext(&hasCurrent));
        nRes++;
    }
    VERIFY_ARE_EQUAL(nRes, 1);

    ComPtr<IAppxManifestReader3> manifestReader3;
    VERIFY_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));
    ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfEnum;
    VERIFY_SUCCEEDED(manifestReader3->GetTargetDeviceFamilies(&tdfEnum));
    hasCurrent = FALSE;
    VERIFY_SUCCEEDED(tdfEnum->GetHasCurrent(&hasCurrent));
    int nTdf = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxManifestTargetDeviceFamily> tdf;
        VERIFY_SUCCEEDED(tdfEnum->GetCurrent(&tdf));
        Text name;
        VERIFY_SUCCEEDED(tdf->GetName(&name));
        VERIFY_WSTR(name.Get(), L"Windows.Universal");
        UINT64 min = 0;
        VERIFY_SUCCEEDED(tdf->GetMinVersion(&min));
        VERIFY_ARE_EQUAL(min, 2814750460870656);
        UINT64 max = 0;
        VERIFY_SUCCEEDED(tdf->GetMaxVersionTested(&max));
        VERIFY_ARE_EQUAL(max, 2814750826954752);
        VERIFY_SUCCEEDED(tdfEnum->MoveNext(&hasCurrent));
        nTdf++;
    }
    VERIFY_ARE_EQUAL(nTdf, 1);

    ComPtr<IMSIXDocumentElement> msixDocument;
    VERIFY_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IMSIXDocumentElement>::iid, reinterpret_cast<void**>(&msixDocument)));
    ComPtr<IMSIXElement> manifestElement;
    VERIFY_SUCCEEDED(msixDocument->GetDocumentElement(&manifestElement));
    Text ignorableNamespaces;
    VERIFY_SUCCEEDED(manifestElement->GetAttributeValue(L"IgnorableNamespaces", &ignorableNamespaces))
    VERIFY_NOT_NULL(ignorableNamespaces.Get());
    VERIFY_WSTR(ignorableNamespaces.Get(), L"uap mp build");

    Text fakeAttribute;
    VERIFY_SUCCEEDED(manifestElement->GetAttributeValue(L"Fake", &fakeAttribute))
    VERIFY_IS_NULL(fakeAttribute.Get());

    ComPtr<IMSIXElementEnumerator> elementEnum;
    VERIFY_SUCCEEDED(manifestElement->GetElements(ApplicationXpath, &elementEnum));
    hasCurrent = FALSE;
    VERIFY_SUCCEEDED(elementEnum->GetHasCurrent(&hasCurrent));
    int nElements = 0;
    while (hasCurrent)
    {
        ComPtr<IMSIXElement> element;
        VERIFY_SUCCEEDED(elementEnum->GetCurrent(&element));
        Text entryPoint;
        VERIFY_SUCCEEDED(element->GetAttributeValue(L"EntryPoint", &entryPoint))
        VERIFY_WSTR(entryPoint.Get(), L"TestAppxPackage.App");
        VERIFY_SUCCEEDED(elementEnum->MoveNext(&hasCurrent));
        nElements++;
    }
    VERIFY_ARE_EQUAL(nElements, 1);

    ComPtr<IAppxManifestPackageId> packageId;
    VERIFY_SUCCEEDED(manifestReader->GetPackageId(&packageId));
    VERIFY_NOT_NULL(packageId.Get());
    TestAppxManifestPackageId(packageId.Get());

    return;
}

void TestPackage()
{
    ComPtr<IAppxFactory> factory;
    ComPtr<IStream> inputStream;
    VERIFY_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(packageToTest), true, &inputStream));
    VERIFY_SUCCEEDED(CoCreateAppxFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        &factory));

    ComPtr<IAppxPackageReader> packageReader;
    VERIFY_SUCCEEDED(factory->CreatePackageReader(inputStream.Get(), &packageReader));

    ComPtr<IAppxFile> manifestFile;
    VERIFY_SUCCEEDED(packageReader->GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE_MANIFEST, &manifestFile));

    Text manifestFileName;
    VERIFY_SUCCEEDED(manifestFile->GetName(&manifestFileName));
    VERIFY_WSTR(manifestFileName.Get(), L"AppxManifest.xml");

    APPX_COMPRESSION_OPTION compressionOpt;
    VERIFY_SUCCEEDED(manifestFile->GetCompressionOption(&compressionOpt));
    VERIFY_ARE_EQUAL(compressionOpt, APPX_COMPRESSION_OPTION_NORMAL);

    UINT64 size;
    VERIFY_SUCCEEDED(manifestFile->GetSize(&size));
    VERIFY_ARE_EQUAL(size, 3251);

    ComPtr<IAppxFilesEnumerator> files;
    VERIFY_SUCCEEDED(packageReader->GetPayloadFiles(&files));
    BOOL hasCurrent = FALSE;
    VERIFY_SUCCEEDED(files->GetHasCurrent(&hasCurrent));
    int nFiles = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxFile> file;
        VERIFY_SUCCEEDED(files->GetCurrent(&file));
        Text fileName;
        VERIFY_SUCCEEDED(file->GetName(&fileName));
        VERIFY_WSTR(fileName.Get(), ExpectedPayloadFiles.at(nFiles));

        ComPtr<IAppxFile> file2;
        VERIFY_SUCCEEDED(packageReader->GetPayloadFile(fileName.Get(), &file2));
        VERIFY_ARE_SAME(file.Get(), file2.Get());
        VERIFY_SUCCEEDED(files->MoveNext(&hasCurrent));
        nFiles++;
    }
    VERIFY_ARE_EQUAL(nFiles, ExpectedPayloadFilesSize);

    ComPtr<IAppxManifestReader> manifestReader;
    VERIFY_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    VERIFY_NOT_NULL(manifestReader.Get());
    TestAppxManifestReader(manifestReader.Get());

    ComPtr<IAppxBlockMapReader> blockMapReader;
    VERIFY_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    VERIFY_NOT_NULL(blockMapReader.Get());
    TestBlockMapReader(blockMapReader.Get());

    return;
}

void TestBundleManifestReader(IAppxBundleManifestReader* manifestReader)
{
    ComPtr<IAppxManifestPackageId> packageInfo;
    VERIFY_SUCCEEDED(manifestReader->GetPackageId(&packageInfo));

    ComPtr<IStream> stream;
    VERIFY_SUCCEEDED(manifestReader->GetStream(&stream));
    VERIFY_NOT_NULL(stream.Get());

    Text fullName;
    VERIFY_SUCCEEDED(packageInfo->GetPackageFullName(&fullName));
    VERIFY_WSTR(fullName.Get(), L"Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe");

    ComPtr<IAppxBundleManifestPackageInfoEnumerator> bundleManifestPackageInfoEnumerator;
    VERIFY_SUCCEEDED(manifestReader->GetPackageInfoItems(&bundleManifestPackageInfoEnumerator));

    BOOL hasCurrent = FALSE;
    VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetHasCurrent(&hasCurrent));
    int nPackageInfo = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxBundleManifestPackageInfo> bundleManifestPackageInfo;
        VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetCurrent(&bundleManifestPackageInfo));

        Text fileName;
        VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetFileName(&fileName));
        VERIFY_WSTR(fileName.Get(), expectedPackagesInBundleList[nPackageInfo].name);

        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
        VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetPackageType(&type));
        VERIFY_ARE_EQUAL(type, expectedPackagesInBundleList[nPackageInfo].type);

        UINT64 offset;
        VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetOffset(&offset));
        VERIFY_ARE_EQUAL(offset, expectedPackagesInBundleList[nPackageInfo].offset);

        UINT64 size;
        VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetSize(&size));
        VERIFY_ARE_EQUAL(size, expectedPackagesInBundleList[nPackageInfo].size);

        ComPtr<IAppxManifestQualifiedResourcesEnumerator> manifestQualifiedResourcesEnumerator;
        VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetResources(&manifestQualifiedResourcesEnumerator));

        BOOL hasCurrentResource = FALSE;
        VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetHasCurrent(&hasCurrentResource));
        int nLanguages = 0;
        while (hasCurrentResource)
        {
            ComPtr<IAppxManifestQualifiedResource> manifestQualifiedResources;
            VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetCurrent(&manifestQualifiedResources));

            Text language;
            VERIFY_SUCCEEDED(manifestQualifiedResources->GetLanguage(&language));
            VERIFY_WSTR(language.Get(), expectedPackagesInBundleList[nPackageInfo].languages[nLanguages]);
            VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->MoveNext(&hasCurrentResource));
            nLanguages++;
        }
        VERIFY_ARE_EQUAL(nLanguages, expectedPackagesInBundleList[nPackageInfo].languages.size());
        VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->MoveNext(&hasCurrent));
        nPackageInfo++;
    }
    VERIFY_ARE_EQUAL(nPackageInfo, expectedPackagesInBundleList.size());

    return;
}

void TestBundle()
{
    ComPtr<IAppxBundleFactory> bundleFactory;
    VERIFY_SUCCEEDED(CoCreateAppxBundleFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPPLATFORM | 
                                               MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE),
        &bundleFactory));

    ComPtr<IAppxBundleReader> bundleReader;

    ComPtr<IStream> badStream;
    VERIFY_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(packageToTest), true, &badStream));
    VERIFY_HR(E_NOINTERFACE, bundleFactory->CreateBundleReader(badStream.Get(), &bundleReader));

    ComPtr<IStream> inputStream;
    VERIFY_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(bundleToTest), true, &inputStream));
    VERIFY_SUCCEEDED(bundleFactory->CreateBundleReader(inputStream.Get(), &bundleReader));

    ComPtr<IAppxFile> bundleManifestFile;
    VERIFY_SUCCEEDED(bundleReader->GetFootprintFile(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST, &bundleManifestFile));

    Text bundleManifestFileName;
    VERIFY_SUCCEEDED(bundleManifestFile->GetName(&bundleManifestFileName));
    VERIFY_WSTR(bundleManifestFileName.Get(), L"AppxMetadata\\AppxBundleManifest.xml");

    ComPtr<IAppxFilesEnumerator> packages;
    VERIFY_SUCCEEDED(bundleReader->GetPayloadPackages(&packages));

    BOOL hasCurrent = FALSE;
    VERIFY_SUCCEEDED(packages->GetHasCurrent(&hasCurrent));
    int nPackage = 0;
    while (hasCurrent)
    {
        ComPtr<IAppxFile> package;
        VERIFY_SUCCEEDED(packages->GetCurrent(&package));
        Text packageName;
        VERIFY_SUCCEEDED(package->GetName(&packageName));
        VERIFY_WSTR(packageName.Get(), expectedPackagesInBundleList[nPackage].name);

        ComPtr<IAppxFile> package2;
        VERIFY_SUCCEEDED(bundleReader->GetPayloadPackage(packageName.Get(), &package2));
        VERIFY_ARE_SAME(package.Get(), package2.Get());
        VERIFY_SUCCEEDED(packages->MoveNext(&hasCurrent));
        nPackage++;
    }
    VERIFY_ARE_EQUAL(nPackage, expectedPackagesInBundleList.size());

    ComPtr<IAppxBundleManifestReader> bundleManifestReader;
    VERIFY_SUCCEEDED(bundleReader->GetManifest(&bundleManifestReader));
    VERIFY_NOT_NULL(bundleManifestReader.Get());
    TestBundleManifestReader(bundleManifestReader.Get());

    return;
}


int main(int argc, char* argv[])
{
    try
    {
        TestPackage();
        TestBundle();
        std::cout << "Test PASSED" << std::endl;
    }
    catch (Exception e)
    {
        std::cout << "Test FAILED: expected: " << e.Expected() << " got: " << e.Actual() << " on line " << e.Line() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cout << "Test FAILED: unexpected failure" << std::endl;
        return -1;
    }
    return 0;
}