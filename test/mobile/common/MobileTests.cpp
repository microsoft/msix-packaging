//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include <cstdlib>
#include <string>
#include <codecvt>
#include <locale>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

#include "MobileTests.hpp"

// Used for test results
bool g_TestFailed = false;

// not all POSIX implementations provide an implementation of mkdirp
static int mkdirp(std::wstring& utf16Path)
{
	std::string utf8Path = utf16_to_utf8(utf16Path);
	auto lastSlash = utf8Path.find_last_of("/");
	std::string path = utf8Path.substr(0, lastSlash);
	char* p = const_cast<char*>(path.c_str());
	if (*p == '/') { p++; }
	while (*p != '\0')
	{   while (*p != '\0' && *p != '/') { p++; }
		char v = *p;
		*p = '\0';
		if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
		{   return errno;
		}
		*p = v;
		p++;
	}
	return 0;
}
// Cleans a directory
static void RemoveContent(std::string subPath)
{
    DIR *dir;
    if ((dir = opendir(subPath.data())))
    {   struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) 
        {   if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {   std::string path = subPath + "/" + std::string(entry->d_name);
                if (entry->d_type == DT_DIR) { RemoveContent(path.data());}
                std::remove(path.data());
            }
        }
        closedir(dir);
    }
}

// allocator/deallocator for non-Windows
LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE MyFree(LPVOID pv)        { std::free(pv); }

class Text
{
public:
    char** operator&() { return &content; }
    ~Text() { Cleanup(); }

    char* content = nullptr;
protected:
    void Cleanup() { if (content) { std::free(content); content = nullptr; } }
};


static HRESULT RunTest(std::string packageName, std::string unpackFolder, MSIX_VALIDATION_OPTION validationOptions, int expectedResult)
{
    HRESULT hr = S_OK;
    RemoveContent(unpackFolder);

    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Package: " << packageName << std::endl;
    std::cout << "Validation Options: " << std::hex << flags << std::endl;

    hr = UnpackPackage(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
            validationOptions,
            const_cast<char*>(packageName),
            const_cast<char*>(unpackFolder);

    if(FAILED(hr))
    {
        std::cout << "Error: " << std::hex << hr << std::endl;
        Text text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {   std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {   std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }

    short result = static_cast<short>(hr);
    std::cout << "Expected: " << std::dec << expectedResult << ", Got: " << result << std::endl;
    if(expectedResult == result)
    {   std::cout << "Succeeded" << std::endl;
    }
    else
    {   std::cout << "Failed" << std::endl;
        g_TestFailed = true;
    }

    return hr;
}

static HRESULT RunTestsInternal(std::string source, std::string target)
{
    HRESULT hr = S_OK;

    // Create output directory
    std::string unpackFolder = target + "unpack";
    if (-1 == mkdir(unpackFolder.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
    {   return HRESULT_FROM_WIN32(errno);
    }

    std::ofstream results(target + "testResults.txt");
    auto oldcout = std::cout.rdbuf(results.rdbuf());

    // Reference from other tests

    MSIX_VALIDATION_OPTION sv = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_VALIDATION_OPTION ss = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_VALIDATION_OPTION full = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;

    // expected result last four digits, but in decimal, not hex.  e.g. 0x8bad0002 == 2, 0x8bad0041 == 65, etc...
    // common codes:
    // SignatureInvalid        = ERROR_FACILITY + 0x0041 == 65
    hr = RunTest(source + "Empty.appx", unpackFolder, sv, 2);
    hr = RunTest(source + "HelloWorld.appx", unpackFolder, ss, 0);
    hr = RunTest(source + "SignatureNotLastPart-ERROR_BAD_FORMAT.appx", unpackFolder, full, 66);
    hr = RunTest(source + "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx", unpackFolder, full, 66);
    hr = RunTest(source + "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx", unpackFolder, sv, 65);
    hr = RunTest(source + "SignedTamperedCD-TRUST_E_BAD_DIGEST.appx", unpackFolder, full, 66);
    hr = RunTest(source + "SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx", unpackFolder, full, 66);
    hr = RunTest(source + "SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx", unpackFolder, full, 66);
    hr = RunTest(source + "SignedUntrustedCert-CERT_E_CHAINING.appx", unpackFolder, full, 66);
    hr = RunTest(source + "StoreSigned_Desktop_x64_MoviesTV.appx", unpackFolder, full, 0);
    hr = RunTest(source + "TestAppxPackage_Win32.appx", unpackFolder, ss, 0);
    hr = RunTest(source + "TestAppxPackage_x64.appx", unpackFolder, ss, 0);
    hr = RunTest(source + "UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx", unpackFolder, full, 18);
    hr = RunTest(source + "FileDoesNotExist.appx", unpackFolder, ss, 1);
    hr = RunTest(source + "BlockMap/Missing_Manifest_in_blockmap.appx", unpackFolder, ss, 81);
    hr = RunTest(source + "BlockMap/ContentTypes_in_blockmap.appx", unpackFolder, ss, 81);
    hr = RunTest(source + "BlockMap/Invalid_Bad_Block.appx", unpackFolder, ss, 81);
    hr = RunTest(source + "BlockMap/Size_wrong_uncompressed.appx", unpackFolder, ss, 81);
    hr = RunTest(source + "BlockMap/HelloWorld.appx", unpackFolder, ss, 0);
    hr = RunTest(source + "BlockMap/Extra_file_in_blockmap.appx", unpackFolder, ss, 2);
    hr = RunTest(source + "BlockMap/File_missing_from_blockmap.appx", unpackFolder, ss, 81);
    hr = RunTest(source + "BlockMap/No_blockmap.appx", unpackFolder, ss, 51);
    hr = RunTest(source + "BlockMap/Bad_Namespace_Blockmap.appx", unpackFolder, ss, 4099);
    hr = RunTest(source + "BlockMap/Duplicate_file_in_blockmap.appx", unpackFolder, ss, 81);

    // Bundle tests
    hr = RunTest(source + "bundles/BlockMapContainsPayloadPackage.appxbundle", unpackFolder, ss, 81);
    hr = RunTest(source + "bundles/BlockMapIsMissing.appxbundle", unpackFolder, ss, 51);
    //hr = RunTest(source + "bundles/BlockMapViolatesSchema.appxbundle", unpackFolder, full,  0);
    //hr = RunTest(source + "bundles/ContainsNeutralAndX86AppPackages.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ContainsNoPayload.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ContainsOnlyResourcePackages.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ContainsTwoNeutralAppPackages.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/MainBundle.appxbundle", unpackFolder, ss, 0 );
    //hr = RunTest(source + "bundles/ManifestDeclaresAppPackageForResourcePackage.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestDeclaresResourcePackageForAppPackage.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestHasExtraPackage.appxbundle", unpackFolder, full, 0);
    hr = RunTest(source + "bundles/ManifestIsMissing.appxbundle", unpackFolder, ss, 52);
    //hr = RunTest(source + "bundles/ManifestPackageHasIncorrectArchitecture.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasIncorrectName.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasIncorrectPublisher.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasIncorrectSize.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasIncorrectVersion.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasInvalidOffset.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestPackageHasInvalidRange.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/ManifestViolatesSchema.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/PayloadPackageHasNonAppxExtension.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/PayloadPackageIsCompressed.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/PayloadPackageIsEmpty.appxbundle.zip", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/PayloadPackageIsNotAppxPackage.appxbundle", unpackFolder, full, 0);
    //hr = RunTest(source + "bundles/PayloadPackageNotListedInManifest.appxbundle", unpackFolder, full, 0);
    hr = RunTest(source + "bundles/SignedUntrustedCert-CERT_E_CHAINING.appxbundle", unpackFolder, full, 66);
    //hr = RunTest(source + "bundles/StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle", unpackFolder, full, 0);

    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    if(g_TestFailed)
    {   std::cout << "                           FAILED                                 " << std::endl;
    }
    else
    {   std::cout << "                           passed                                 " << std::endl;
    }

    std::cout.rdbuf(oldcout);

    return S_OK;
}

__attribute__((visibility("default"))) signed long RunTests(char* source, char* target)
{
    return static_cast<signed long>(RunTestsInternal(source, target));
}