//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <iostream>
#include <vector>
#include <map>

#ifdef WIN32
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#else 
#include <queue>
#include <fts.h>
#include <dirent.h>
#endif

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include "Helpers.hpp"

using namespace MsixSample::Helper;

int Help()
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "\t" << "PackSample -d <directory> -p <output package> " << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tSample to show the usage of the AppxPackaging APIs. Takes a directory and" << std::endl;
    std::cout << "\tcreates an app package using the AppxPackaging APIs. The sample assumes "  << std::endl;
    std::cout << "\t the AppxManifest.xml is in the directory provided."                       << std::endl;
    std::cout << std::endl;
    return 0;
}

// This is the same "cheat code" we use internally if no compression options is provided
// you can use your own logic for which files to compress or not.
// In this sample, if an extension is not listed here we default to APPX_COMPRESSION_OPTION_NORMAL
static const std::map<std::string, APPX_COMPRESSION_OPTION> extToContentType = 
{
    { "atom",  APPX_COMPRESSION_OPTION_NORMAL },
    { "appx",  APPX_COMPRESSION_OPTION_NONE },
    { "b64",   APPX_COMPRESSION_OPTION_NORMAL },
    { "cab",   APPX_COMPRESSION_OPTION_NONE },
    { "doc",   APPX_COMPRESSION_OPTION_NORMAL },
    { "dot",   APPX_COMPRESSION_OPTION_NORMAL },
    { "docm",  APPX_COMPRESSION_OPTION_NONE },
    { "docx",  APPX_COMPRESSION_OPTION_NONE },
    { "dotm",  APPX_COMPRESSION_OPTION_NONE },
    { "dotx",  APPX_COMPRESSION_OPTION_NONE },
    { "dll",   APPX_COMPRESSION_OPTION_NORMAL },
    { "dtd",   APPX_COMPRESSION_OPTION_NORMAL },
    { "exe",   APPX_COMPRESSION_OPTION_NORMAL },
    { "gz",    APPX_COMPRESSION_OPTION_NONE },
    { "java",  APPX_COMPRESSION_OPTION_NORMAL },
    { "json",  APPX_COMPRESSION_OPTION_NORMAL },
    { "p7s",   APPX_COMPRESSION_OPTION_NORMAL },
    { "pdf",   APPX_COMPRESSION_OPTION_NORMAL },
    { "ps",    APPX_COMPRESSION_OPTION_NORMAL },
    { "potm",  APPX_COMPRESSION_OPTION_NONE },
    { "potx",  APPX_COMPRESSION_OPTION_NONE },
    { "ppam",  APPX_COMPRESSION_OPTION_NONE },
    { "ppsm",  APPX_COMPRESSION_OPTION_NONE },
    { "ppsx",  APPX_COMPRESSION_OPTION_NONE },
    { "ppt",   APPX_COMPRESSION_OPTION_NORMAL },
    { "pot",   APPX_COMPRESSION_OPTION_NORMAL },
    { "pps",   APPX_COMPRESSION_OPTION_NORMAL },
    { "ppa",   APPX_COMPRESSION_OPTION_NORMAL },
    { "pptm",  APPX_COMPRESSION_OPTION_NONE },
    { "pptx",  APPX_COMPRESSION_OPTION_NONE },
    { "rar",   APPX_COMPRESSION_OPTION_NONE },
    { "rss",   APPX_COMPRESSION_OPTION_NORMAL },
    { "soap",  APPX_COMPRESSION_OPTION_NORMAL },
    { "tar",   APPX_COMPRESSION_OPTION_NONE },
    { "xaml",  APPX_COMPRESSION_OPTION_NORMAL },
    { "xap",   APPX_COMPRESSION_OPTION_NONE },
    { "xbap",  APPX_COMPRESSION_OPTION_NORMAL },
    { "xhtml", APPX_COMPRESSION_OPTION_NORMAL },
    { "xlam",  APPX_COMPRESSION_OPTION_NONE },
    { "xls",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xlt",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xla",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xlsb",  APPX_COMPRESSION_OPTION_NONE },
    { "xlsm",  APPX_COMPRESSION_OPTION_NONE },
    { "xlsx",  APPX_COMPRESSION_OPTION_NONE },
    { "xltm",  APPX_COMPRESSION_OPTION_NONE },
    { "xltx",  APPX_COMPRESSION_OPTION_NONE },
    { "xsl",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xslt",  APPX_COMPRESSION_OPTION_NORMAL },
    { "zip",   APPX_COMPRESSION_OPTION_NONE },
    // Text types
    { "c",     APPX_COMPRESSION_OPTION_NORMAL },
    { "cpp",   APPX_COMPRESSION_OPTION_NORMAL },
    { "cs",    APPX_COMPRESSION_OPTION_NORMAL },
    { "css",   APPX_COMPRESSION_OPTION_NORMAL },
    { "csv",   APPX_COMPRESSION_OPTION_NORMAL },
    { "h",     APPX_COMPRESSION_OPTION_NORMAL },
    { "htm",   APPX_COMPRESSION_OPTION_NORMAL },
    { "html",  APPX_COMPRESSION_OPTION_NORMAL },
    { "js",    APPX_COMPRESSION_OPTION_NORMAL },
    { "rtf",   APPX_COMPRESSION_OPTION_NORMAL },
    { "sct",   APPX_COMPRESSION_OPTION_NORMAL },
    { "txt",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xml",   APPX_COMPRESSION_OPTION_NORMAL },
    { "xsd",   APPX_COMPRESSION_OPTION_NORMAL },
    // Audio types
    { "aiff",  APPX_COMPRESSION_OPTION_NORMAL },
    { "au",    APPX_COMPRESSION_OPTION_NORMAL },
    { "m4a",   APPX_COMPRESSION_OPTION_NONE },
    { "mid",   APPX_COMPRESSION_OPTION_NORMAL },
    { "mp3",   APPX_COMPRESSION_OPTION_NONE },
    { "smf",   APPX_COMPRESSION_OPTION_NORMAL },
    { "wav",   APPX_COMPRESSION_OPTION_NORMAL },
    { "wma",   APPX_COMPRESSION_OPTION_NONE },
    // Image types
    { "bmp",   APPX_COMPRESSION_OPTION_NORMAL },
    { "emf",   APPX_COMPRESSION_OPTION_NORMAL },
    { "gif",   APPX_COMPRESSION_OPTION_NONE },
    { "ico",   APPX_COMPRESSION_OPTION_NORMAL },
    { "jpg",   APPX_COMPRESSION_OPTION_NONE },
    { "jpeg",  APPX_COMPRESSION_OPTION_NONE },
    { "png",   APPX_COMPRESSION_OPTION_NONE },
    { "svg",   APPX_COMPRESSION_OPTION_NORMAL },
    { "tif",   APPX_COMPRESSION_OPTION_NORMAL },
    { "tiff",  APPX_COMPRESSION_OPTION_NORMAL },
    { "wmf",   APPX_COMPRESSION_OPTION_NORMAL },
    // Video types
    { "avi",   APPX_COMPRESSION_OPTION_NONE },
    { "mpeg",  APPX_COMPRESSION_OPTION_NONE },
    { "mpg",   APPX_COMPRESSION_OPTION_NONE },
    { "mov",   APPX_COMPRESSION_OPTION_NONE },
    { "wmv",   APPX_COMPRESSION_OPTION_NONE }
};

#ifdef WIN32
std::vector<std::string> GetAllFilesInDirectory(const std::string& directory)
{
    std::vector<std::string> files;
    for (const auto& file : recursive_directory_iterator(directory))
    {
        if (!is_directory(file))
        {
            files.push_back(file.path().string());
        }
    }
    return files;
}
#else
std::vector<std::string> GetAllFilesInDirectory(const std::string& directory)
{
    std::vector<std::string> files;
    std::queue<std::string> directories;
    directories.push(directory);

    static std::string dot(".");
    static std::string dotdot("..");
    do
    {
        auto root = directories.front();
        directories.pop();
        std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(root.c_str()), closedir);
        if (dir.get() != nullptr)
        {
            struct dirent* dp;
            while((dp = readdir(dir.get())) != nullptr)
            {
                std::string fileName = std::string(dp->d_name);
                std::string child = root + "/" + fileName;
                if (dp->d_type == DT_DIR)
                {
                    if ((fileName != dot) && (fileName != dotdot))
                    {
                        directories.push(child);
                    }
                }
                else
                {
                    files.push_back(child);
                }
            }
        }
    } while (!directories.empty());
    return files;
}
#endif

// Add payload files to the package and returns the AppxManifest stream
HRESULT AddPayloadFilesAndGetManifestStream(IAppxPackageWriterUtf8* packageWriter, const std::string& directory, IStream** appxManifestStream)
{
    *appxManifestStream = nullptr;
    auto payloadFiles = GetAllFilesInDirectory(directory);
    for (const auto& file : payloadFiles)
    {
        // Remove the top level directory from the name
        std::string name = file.substr(directory.size()+1);
        if (!IsFootPrintFile(name))
        {
            std::cout << "Packing payload file: "  << name << std::endl;
            ComPtr<IStream> stream;
            RETURN_IF_FAILED(CreateStreamOnFile(const_cast<char*>(file.c_str()), true, &stream));

            std::string ext = name.substr(name.find_last_of('.')+1);
            APPX_COMPRESSION_OPTION compressOpt = APPX_COMPRESSION_OPTION_NORMAL; // default
            auto compression = extToContentType.find(ext);
            if (compression != extToContentType.end())
           {
                compressOpt = compression->second;
            }

            RETURN_IF_FAILED(packageWriter->AddPayloadFile(
                name.c_str(),
                "application/fake-content-type", // sample :)
                compressOpt,
                stream.Get()
            ));
        }
        else if(IsAppxManifest(name))
        {
            RETURN_IF_FAILED(CreateStreamOnFile(const_cast<char*>(file.c_str()), true, appxManifestStream));
        }
    }

    return S_OK;
}

HRESULT PackPackage(const std::string& package, const std::string& directory)
{
     // Initialize the factory.
    ComPtr<IAppxFactory> appxFactory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        &appxFactory));

    // Create output stream
    ComPtr<IStream> outputStream;
    RETURN_IF_FAILED(CreateStreamOnFile(const_cast<char*>(package.c_str()), false, &outputStream));

    // Create the package writer. Note, APPX_PACKAGE_SETTINGS is not implemented for the MSIX SDK
    // we always create a zip64 and use SHA256 as hash method.
    ComPtr<IAppxPackageWriter> packageWriter;
    RETURN_IF_FAILED(appxFactory->CreatePackageWriter(outputStream.Get(), nullptr, &packageWriter));

    // The MSIX SDK includes Ut8 variants for convenience for non-Windows platforms. We use them in this sample.
    ComPtr<IAppxPackageWriterUtf8> packageWriterUtf8;
    RETURN_IF_FAILED(packageWriter->QueryInterface(UuidOfImpl<IAppxPackageWriterUtf8>::iid, reinterpret_cast<void**>(&packageWriterUtf8)));

    // The sample assumes that AppxManifest.xml is in the directory.
    ComPtr<IStream> manifestStream;
    RETURN_IF_FAILED(AddPayloadFilesAndGetManifestStream(packageWriterUtf8.Get(), directory, &manifestStream));
    if (manifestStream.Get() == nullptr)
    {
        std::cout << "Error: AppxManifest.xml not found" << std::endl;
        return E_FAIL;
    }

    // Close the package
    RETURN_IF_FAILED(packageWriter->Close(manifestStream.Get()));

    return S_OK;
}

int main(int argc, char* argv[])
{
    int index = 1;
    std::string package;
    std::string directory;
    while (index < argc)
    {
        if (strcmp(argv[index], "-p") == 0)
        {
            package = argv[++index];
            index++;
        }
        else if (strcmp(argv[index], "-d") == 0)
        {
            directory = argv[++index];
            index++;
        }
        else
        {
            return Help();
        }
    }
    if (package.empty() || directory.empty())
    {
        return Help();
    }

    HRESULT hr = PackPackage(package, directory);
    
    if (hr != S_OK)
    {
        remove(package.c_str());
        std::cout << "Error: " << std::hex << hr << " while extracting the appx package" <<std::endl;
        Text<char> text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {
            std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {
            std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }
    else
    {
        std::cout << "Success: package " << package << " created" << std::endl;
    }

    return 0;
}
