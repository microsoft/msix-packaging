#include <windows.h>
#include <iostream>
#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name

#include "Extractor.h"
#include "GeneralUtil.h"
#include "FootprintFiles.h"
#include "FilePaths.h"
#include "Uninstall.h"
#include "InstallUI.h"

//
// Replaces all oldchars in input with newchar
//
// Parameters:
// input   - The input string that contains the characters to be changed
// oldchar - Old character that are to be replaced
// newchar - New character that replaces oldchar
//
void replace(std::wstring& input, const wchar_t oldchar, const wchar_t newchar)
{
    std::size_t found = input.find_first_of(oldchar);
    while (found != std::string::npos)
    {
        input[found] = newchar;
        found = input.find_first_of(oldchar, found + 1);
    }
}

//
// Makes a directory based on the inputted filepath
// 
// Parameters:
// utf16Path - The filepath to create a directory in utf16
// 
int mkdirp(std::wstring& utf16Path)
{
    replace(utf16Path, L'/', L'\\');
    for (std::size_t i = 3; i < utf16Path.size(); i++) // 3 skips past c:
    {
        if (utf16Path[i] == L'\0')
        {
            break;
        }
        else if (utf16Path[i] == L'\\')
        {
            // Temporarily set string to terminate at the '\' character
            // to obtain name of the subdirectory to create
            utf16Path[i] = L'\0';

            if (!CreateDirectoryW(utf16Path.c_str(), nullptr))
            {
                int lastError = static_cast<int>(GetLastError());

                // It is normal for CreateDirectory to fail if the subdirectory
                // already exists.  Other errors should not be ignored.
                if (lastError != ERROR_ALREADY_EXISTS)
                {
                    return lastError;
                }
            }
            // Restore original string
            utf16Path[i] = L'\\';
        }
    }
    return 0;
}

//
// Helper function to create a writable IStream over a file with the specified name
// under the specified path.  This function will also create intermediate
// subdirectories if necessary.  
//
// Parameters:
// path        - Path of the folder containing the file to be opened.  This should NOT
//            end with a slash ('\') character.
// fileName - Name, not including path, of the file to be opened
// stream    - Output parameter pointing to the created instance of IStream over
//              the specified file when this function succeeds.
//
HRESULT GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream)
{
    std::wstring fullFileName = path + std::wstring(L"\\") + fileName;
    RETURN_IF_FAILED(HRESULT_FROM_WIN32(mkdirp(fullFileName)));
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(fullFileName.c_str(), false, stream));
    return S_OK;
}

//
// Prints basic info about a footprint or payload file and writes the file to disk.
//
// Parameters:
// file          -    The IAppxFile interface that represents a footprint or payload file 
//                in the package.
// outputPath - The path of the folder for the extracted files.
//
HRESULT ExtractFile(IAppxFile* file, LPCWSTR outputPath)
{
    Text<WCHAR> fileName;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    // Get basic info about the file
    RETURN_IF_FAILED(file->GetName(&fileName));
    RETURN_IF_FAILED(file->GetSize(&fileSize));
    fileSizeLargeInteger.QuadPart = fileSize;

    // Write the file to disk
    RETURN_IF_FAILED(file->GetStream(&fileStream));
    RETURN_IF_FAILED(GetOutputStream(outputPath, fileName.Get(), &outputStream));
    RETURN_IF_FAILED(fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr));
    return S_OK;
}

//
// Extracts all footprint files from a package.
//
// Parameters:
// packageReader - The package reader for the app package.
// outputPath    - The path of the folder for the extracted footprint files.
//
HRESULT ExtractFootprintFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    std::printf("Extracting footprint files from the package...\n");
    for (int i = 0; i < FootprintFilesCount; i++)
    {
        ComPtr<IAppxFile> footprintFile;
        HRESULT hr = package->GetFootprintFile(g_footprintFilesType[i].fileType, &footprintFile);
        if (SUCCEEDED(hr) && footprintFile.Get())
        {
            RETURN_IF_FAILED(ExtractFile(footprintFile.Get(), outputPath));
        }
        else if (g_footprintFilesType[i].isRequired)
        {
            std::printf("The package does not contain a %s.\n", g_footprintFilesType[i].description);
            return hr;
        }
    }
    return S_OK;
}

//
// Extracts all payload files from a package.
//
// Parameters:
// packageReader - The package reader for the app package.
// outputPath     - The path of the folder for the extracted payload files.
//
HRESULT ExtractPayloadFiles(IAppxPackageReader* package, LPCWSTR outputPath, TrackerXML* myXml)
{
    ComPtr<IAppxFilesEnumerator> files;
    std::printf("Extracting payload files from the package...\n");

    // Get an enumerator of all payload files from the package reader and iterate
    // through all files.
    RETURN_IF_FAILED(package->GetPayloadFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));

    while (hasCurrent)
    {
        ComPtr<IAppxFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));
        Text<WCHAR> name;
        RETURN_IF_FAILED(file->GetName(&name));
        std::wstring nameStr = name.Get();
        
        std::map<std::wstring, std::wstring> map = GetMap();
        if (nameStr.find(L"VFS") != std::wstring::npos)
        {
            ExtractVFSToLocal(file.Get(), nameStr, myXml);
        }
        else
        {
            RETURN_IF_FAILED(ExtractFile(file.Get(), outputPath));
        }
        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
        UpdateProgressBar();
    }

    return S_OK;
}

//
// Extracts all files from a package.
//
// Parameters:
// package      - The package reader for the app package.
// outputPath - The path of the folder for the extracted payload files.
//
HRESULT ExtractPackage(IAppxPackageReader* package, LPCWSTR outputPath, TrackerXML* myXml)
{
    // Print information about all footprint files, and extract them to disk
    RETURN_IF_FAILED(ExtractFootprintFiles(package, outputPath));
    // Print information about all payload files, and extract them to disk
    RETURN_IF_FAILED(ExtractPayloadFiles(package, outputPath, myXml));
    return S_OK;
}


//
// Extracts a file stored in the VFS to the disk.
//
// Parameters:
// file       - The IAppxFile interface that represents a footprint or payload file in the package.
// outputPath - The path of the folder for the extracted files.
//
HRESULT ExtractVFSFile(IAppxFile* file, LPCWSTR outputPath)
{
    Text<WCHAR> fileName;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    // Get basic info about the file
    RETURN_IF_FAILED(file->GetName(&fileName));

    std::wstring VFSFileName = std::wstring(fileName.Get());
    std::wstring extractFile;

    //Gets only the file and none of the super directories
    while (VFSFileName.back() != '\\')
    {
        extractFile.push_back(VFSFileName.back());
        VFSFileName.erase(VFSFileName.end() - 1, VFSFileName.end());
    }

    std::reverse(extractFile.begin(), extractFile.end());

    RETURN_IF_FAILED(file->GetSize(&fileSize));
    fileSizeLargeInteger.QuadPart = fileSize;

    // Write the file to disk
    RETURN_IF_FAILED(file->GetStream(&fileStream));
    RETURN_IF_FAILED(GetOutputStream(outputPath, extractFile.c_str(), &outputStream));
    RETURN_IF_FAILED(fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr));
    return S_OK;
}

//
// A helper function that takes in a filepath from the VFS and a COM pointer to an IAppxFile
// and extracts the IAppxFile to the local computer based on the correct KnownFolderID from
// the GetMap()
// 
// Parameters:
// file       - A COM pointer to an IAppxFile that is obtained from a MSIX/APPX package
// nameStr - A filepath of the file in the VFS (e.g. VFS\Program Files (x86)\Notepadplusplus\notepadplusplus.exe)
//
void ExtractVFSToLocal(IAppxFile* file, std::wstring nameStr, TrackerXML* myXML)
{
    std::map<std::wstring, std::wstring> map = GetMap();

    for (auto& pair : map)
    {
        if (nameStr.find(pair.first) != std::wstring::npos)
        {
            //The following code gets from "VFS/FirstDir/.../file.ext" to "/.../"
            std::wstring remainingFilePath = nameStr;

            //Erases the first two parent directories
            GetPathChild(remainingFilePath);
            GetPathChild(remainingFilePath);

            //Initialize the full local path
            std::wstring localPath = pair.second;
            localPath.push_back(L'\\');
            localPath.append(remainingFilePath);

            //Creates an XML that has the files paths of all the files to be deleted
            myXML->AddFileToRecord(localPath);

            //Erases the file at the end of the path 
            GetPathParent(localPath);

            //Extract the file directly to its current directory
            ExtractVFSFile(file, localPath.c_str());

            //Stop looping through the list
            return;
        }
    }
}