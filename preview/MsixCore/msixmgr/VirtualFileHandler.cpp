#include <windows.h>
#include <iostream>
#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name

#include "VirtualFileHandler.hpp"
#include "GeneralUtil.hpp"
#include "FootprintFiles.hpp"
#include "FilePaths.hpp"
#include "Constants.hpp"

#include "RegistryDevirtualizer.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"

using namespace MsixCoreLib;

const PCWSTR VirtualFileHandler::HandlerName = L"VirtualFileHandler";

HRESULT VirtualFileHandler::ExecuteForAddRequest()
{
    auto vfsDirectoryPath = m_msixRequest->GetPackageDirectoryPath() + L"\\VFS";

    for (auto& p : std::experimental::filesystem::recursive_directory_iterator(vfsDirectoryPath))
    {
        if (std::experimental::filesystem::is_regular_file(p.path()))
        {
            RETURN_IF_FAILED(CopyVfsFileToLocal(p.path()));
        }
    }

    return S_OK;
}

HRESULT VirtualFileHandler::RemoveVfsFiles()
{
    std::wstring blockMapPath = m_msixRequest->GetPackageDirectoryPath() + blockMapFile;
    ComPtr<IStream> stream;
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(blockMapPath.c_str(), true /*forRead*/, &stream));

    ComPtr<IAppxFactory> appxFactory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, m_msixRequest->GetValidationOptions(), &appxFactory));

    ComPtr<IAppxBlockMapReader> blockMapReader;
    RETURN_IF_FAILED(appxFactory->CreateBlockMapReader(stream.Get(), &blockMapReader));

    ComPtr<IAppxBlockMapFilesEnumerator> files;
    RETURN_IF_FAILED(blockMapReader->GetFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));

    while (hasCurrent)
    {
        ComPtr<IAppxBlockMapFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));

        //if it's a VFS file, delete it from the local location
        Text<WCHAR> name;
        RETURN_IF_FAILED(file->GetName(&name));
        std::wstring nameStr = name.Get();
        if (nameStr.find(L"VFS") != std::wstring::npos)
        {
            RETURN_IF_FAILED(RemoveVfsFile(nameStr));
        }

        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT VirtualFileHandler::ExecuteForRemoveRequest()
{
    HRESULT hrRemoveVfsFiles = RemoveVfsFiles();
    if (FAILED(hrRemoveVfsFiles))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to remove VFS files",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrRemoveVfsFiles, "HR"));
    }
    
    return S_OK;
}

HRESULT VirtualFileHandler::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<VirtualFileHandler> localInstance(new VirtualFileHandler(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    RETURN_IF_FAILED(localInstance->m_sharedDllsKey.Open(HKEY_LOCAL_MACHINE, sharedDllsKeyPath.c_str(), KEY_READ | KEY_WRITE));

    *instance = localInstance.release();

    return S_OK;
}

HRESULT GetFileVersion(std::wstring file, _Out_ UINT64& version, _Out_ bool& isUnversioned)
{
    isUnversioned = true;
    DWORD size = GetFileVersionInfoSize(file.c_str(), nullptr);
    if (size == 0)
    {
        DWORD error = GetLastError();
        if (error == ERROR_RESOURCE_DATA_NOT_FOUND || error == ERROR_RESOURCE_TYPE_NOT_FOUND)
        {
            // Does not have version info, isUnversioned = true was set earlier.
            version = 0;
            return S_OK;
        }
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(error));
    }

    std::unique_ptr<BYTE[]> versionInfo(new BYTE[size]);
    if (!GetFileVersionInfo(file.c_str(), 0, size, versionInfo.get()))
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT fileInfoLength = 0;
    if (!VerQueryValue(versionInfo.get(), TEXT("\\"), (LPVOID*)&fileInfo, &fileInfoLength))
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    version = ((UINT64)(fileInfo->dwFileVersionMS) << 32) + fileInfo->dwFileVersionLS;
    isUnversioned = false;

    return S_OK;
}

HRESULT IsFileModified(std::wstring file, _Out_ bool& isModified)
{
    isModified = false;
    // Since we follow MSI file rules, the file is considered modified by MSI if modified date is more than 2 seconds later than creation date 
    const int TwoSecondsInFileTimeIncrements = 20000000; // FILETIME is in 100 nanosecond increments

    WIN32_FILE_ATTRIBUTE_DATA strData;
    if (!GetFileAttributesEx(file.c_str(), GetFileExInfoStandard, (LPVOID)&strData))
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    if ((strData.ftLastWriteTime.dwHighDateTime > strData.ftCreationTime.dwHighDateTime))
    {
        // This copies the MSI bug where as long as LastWrite dwHighDateTime is greater, it's considered modified, even if
        // the Create/LastWrite are within 2 seconds of one another (by virtue of creation dwLowDateTime being near UINT_MAX)
        isModified = true;
    }
    else if (strData.ftLastWriteTime.dwHighDateTime == strData.ftCreationTime.dwHighDateTime)
    {
        if (strData.ftLastWriteTime.dwLowDateTime > strData.ftCreationTime.dwLowDateTime &&
            strData.ftLastWriteTime.dwLowDateTime - strData.ftCreationTime.dwLowDateTime > TwoSecondsInFileTimeIncrements)
        {
            isModified = true;
        }
    }

    return S_OK;
}

HRESULT VirtualFileHandler::NeedToCopyFile(std::wstring sourceFullPath, std::wstring targetFullPath, _Out_ bool &needToCopyFile)
{
    needToCopyFile = false;

    bool targetFileExists = false;
    RETURN_IF_FAILED(FileExists(targetFullPath, targetFileExists));

    if (!targetFileExists)
    {
        needToCopyFile = true;
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Need to copy file because target doesn't exist",
            TraceLoggingValue(targetFullPath.c_str(), "TargetFullPath"));
        return S_OK;
    }

    if (!m_msixRequest->IsReinstall())
    {
        // The file already exists, so we need to refcount it regardless of whether we need to overwrite the existing file or not
        // Cases for incrementing SharedDLLs. For simplicity, we ignore the count in registry.dat as we treat the .msix package as having one reference to the file
        // 
        // File exists, already exists in current system SharedDLLs => increment current SharedDLLs + 1
        // File exists, not in current system SharedDLLs => create new SharedDLLs = 2 (the one existing + this package)
        UINT32 count = 0;
        bool sharedDllValueExists = false;
        bool shouldDelete = true;
        RETURN_IF_FAILED(m_sharedDllsKey.GetUInt32ValueIfExists(targetFullPath.c_str(), count, sharedDllValueExists));
        if (sharedDllValueExists)
        {
            count++;
        }
        else
        {
            count = 2; // implicitly 1 for the file already existing, +1 for the package we're currently installing
        }

        RETURN_IF_FAILED(m_sharedDllsKey.SetUInt32Value(targetFullPath.c_str(), count));
    }

    // Whether we overwrite existing files or keep the existing file as-is follows MSI file versioning rules
    UINT64 targetFileVersion = 0;
    bool targetFileIsUnversioned = false;
    bool targetFileIsModified = false;
    RETURN_IF_FAILED(GetFileVersion(targetFullPath, targetFileVersion, targetFileIsUnversioned));
    if (targetFileIsUnversioned)
    {
        RETURN_IF_FAILED(IsFileModified(targetFullPath, targetFileIsModified));
    }

    UINT64 sourceFileVersion = 0;
    bool sourceFileIsUnversioned = false;
    RETURN_IF_FAILED(GetFileVersion(sourceFullPath, sourceFileVersion, sourceFileIsUnversioned));

    std::wstring targetVersionString = ConvertVersionToString(targetFileVersion);
    std::wstring sourceVersionString = ConvertVersionToString(sourceFileVersion);

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Target Exists, file versioning information",
        TraceLoggingValue(targetFullPath.c_str(), "TargetFullPath"),
        TraceLoggingValue(targetVersionString.c_str(), "TargetFileVersion"),
        TraceLoggingValue(targetFileIsUnversioned, "TargetFileIsUnversioned"),
        TraceLoggingValue(targetFileIsModified, "TargetFileIsModified"),
        TraceLoggingValue(sourceFullPath.c_str(), "SourceFullPath"),
        TraceLoggingValue(sourceVersionString.c_str(), "SourceFileVersion"),
        TraceLoggingValue(sourceFileIsUnversioned, "SourceFileIsUnversioned"));

    if (targetFileIsUnversioned && !sourceFileIsUnversioned)
    {
        // Versioned file should overwrite unversioned file
        needToCopyFile = true;
        return S_OK;
    }

    if (targetFileIsUnversioned && !targetFileIsModified)
    {
        // Unversioned file is treated as userData; we do not want to delete user customizations if they made changes to the file
        // Existing file is unversioned and unmodified -- this is treated as unmodified user data so we can overwrite it
        needToCopyFile = true;
        return S_OK;
    }

    if (targetFileVersion < sourceFileVersion)
    {
        // Higher version wins
        needToCopyFile = true;
        return S_OK;
    }

    return S_OK;
}

HRESULT VirtualFileHandler::CopyVfsFileIfNecessary(std::wstring sourceFullPath, std::wstring targetFullPath)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "CopyVfsFile",
        TraceLoggingValue(sourceFullPath.c_str(), "Source"),
        TraceLoggingValue(targetFullPath.c_str(), "Target"));

    bool needToCopyFile = false;
    RETURN_IF_FAILED(NeedToCopyFile(sourceFullPath, targetFullPath, needToCopyFile));

    if (needToCopyFile)
    {
        HRESULT hrMkdir = HRESULT_FROM_WIN32(mkdirp(targetFullPath));
        if (FAILED(hrMkdir))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to create directory for copying file",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(targetFullPath.c_str(), "FullPath"),
                TraceLoggingValue(hrMkdir, "HR"));
        }
        else if (!CopyFile(sourceFullPath.c_str(), targetFullPath.c_str(), FALSE /*failIfExists*/))
        {
            DWORD error = GetLastError();
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to Copy file",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(targetFullPath.c_str(), "FullPath"),
                TraceLoggingValue(error, "error"));
        }
    }

    return S_OK;
}

HRESULT VirtualFileHandler::RemoveVfsFile(std::wstring fileName)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "RemoveVfsFile",
        TraceLoggingValue(fileName.c_str(), "FileName"));

    std::wstring fullPath;
    if (FAILED(ConvertVfsNameToFullPath(fileName, fullPath)))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Could not find VFS mapping",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(fileName.c_str(), "fileName"));
        return S_OK;
    }

    // Check if file is referenced in SharedDLLs key.
    // If it is, then just decrement instead of delete; or delete the reg value if it's 0 after decrement
    UINT32 count = 0;
    bool sharedDllValueExists = false;
    bool shouldDelete = true;
    HRESULT hrGetUInt32Value = m_sharedDllsKey.GetUInt32ValueIfExists(fullPath.c_str(), count, sharedDllValueExists);
    if (FAILED(hrGetUInt32Value))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to determine if file is shared -- not deleting file",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(fullPath.c_str(), "FullPath"),
            TraceLoggingValue(hrGetUInt32Value, "hr"));
        shouldDelete = false;
    }
    else if (sharedDllValueExists)
    {
        if (count > 1)
        {
            shouldDelete = false;
            HRESULT hrSetUInt32Value = m_sharedDllsKey.SetUInt32Value(fullPath.c_str(), count - 1);
            if (FAILED(hrSetUInt32Value))
            {
                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "Unable to decrement sharedDLL key",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(fullPath.c_str(), "FullPath"),
                    TraceLoggingValue(hrSetUInt32Value, "hr"));
            }
        }
        else
        {
            HRESULT hrDeleteValue = m_sharedDllsKey.DeleteValue(fullPath.c_str());
            if (FAILED(hrDeleteValue))
            {
                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "Unable to delete sharedDLL key",
                    TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                    TraceLoggingValue(fullPath.c_str(), "FullPath"),
                    TraceLoggingValue(hrDeleteValue, "hr"));
                shouldDelete = false;
            }
        }
    }

    if (shouldDelete)
    {
        bool success = DeleteFile(fullPath.c_str());
        if (!success)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to Delete file",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(fullPath.c_str(), "FullPath"),
                TraceLoggingValue(GetLastError(), "error"));
        }

        while (success)
        {
            MsixCoreLib_GetPathParent(fullPath);

            // instead of checking if the directory is empty, just try to delete it.
            // if it's not empty it'll fail with expected error code that we can ignore
            success = RemoveDirectory(fullPath.c_str());
            if (!success)
            {
                DWORD error = GetLastError();
                if (error != ERROR_DIR_NOT_EMPTY)
                {
                    TraceLoggingWrite(g_MsixTraceLoggingProvider,
                        "Unable to Delete directory",
                        TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                        TraceLoggingValue(fullPath.c_str(), "FullPath"),
                        TraceLoggingValue(GetLastError(), "error"));
                }
            }
            // if we're successfull in deleting the directory, try to delete the containing directory too, in case it's now empty
        }
    }
    return S_OK;
}

HRESULT VirtualFileHandler::ConvertVfsNameToFullPath(std::wstring fileName, std::wstring& targetFullPath)
{
    //Convert filename "VFS\FirstDir\...\file.ext" to remainingFilePath  to "FirstDir\...\file.ext"
    std::wstring remainingFilePath = fileName;
    MsixCoreLib_GetPathChild(remainingFilePath); // remove the VFS directory
    RETURN_IF_FAILED(ConvertRemainingPathToFullPath(remainingFilePath, targetFullPath));

    return S_OK;
}

HRESULT VirtualFileHandler::ConvertVfsFullPathToFullPath(std::wstring sourceFullPath, std::wstring& targetFullPath)
{
    //Convert sourceFullPath "c:\program files\MsixCoreApps\<package>\VFS\FirstDir\...\file.ext" to remainingFilePath  to "FirstDir\...\file.ext"
    std::wstring vfsDirectoryPath = m_msixRequest->GetPackageDirectoryPath() + L"\\VFS\\";
    std::wstring remainingFilePath = sourceFullPath.substr(vfsDirectoryPath.size(), sourceFullPath.size());
    RETURN_IF_FAILED(ConvertRemainingPathToFullPath(remainingFilePath, targetFullPath));

    return S_OK;
}

HRESULT VirtualFileHandler::ConvertRemainingPathToFullPath(std::wstring& remainingFilePath, std::wstring& targetFullPath)
{
    std::map<std::wstring, std::wstring> map = FilePathMappings::GetInstance().GetMap();

    std::wstring virtualDirectory = remainingFilePath.substr(0, remainingFilePath.find_first_of(L'\\'));
    auto it = map.find(virtualDirectory);
    if (it != map.end())
    {
        MsixCoreLib_GetPathChild(remainingFilePath); // remove the FirstDir directory.

        // Pre-pend the VFS target directory to obtain the full path for the target location
        targetFullPath = it->second + std::wstring(L"\\") + remainingFilePath;

        //Stop looping through the list
        return S_OK;
    }

    return E_NOT_SET;
}

HRESULT VirtualFileHandler::CopyVfsFileToLocal(std::wstring sourceFullPath)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "CopyVfsFileToLocal",
        TraceLoggingValue(sourceFullPath.c_str(), "FileName"));
    
    std::wstring targetFullPath;
    if (FAILED(ConvertVfsFullPathToFullPath(sourceFullPath, targetFullPath)))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Could not find VFS mapping",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(sourceFullPath.c_str(), "SourceFullPath"));
        return S_OK;
    }

    RETURN_IF_FAILED(CopyVfsFileIfNecessary(sourceFullPath, targetFullPath));

    return S_OK;
}