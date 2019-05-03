#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <experimental/filesystem> // C++-standard header file name

#include "PrepareDevirtualizedRegistry.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"
#include "RegistryDevirtualizer.hpp"

using namespace MsixCoreLib;

const PCWSTR PrepareDevirtualizedRegistry::HandlerName = L"PrepareDevirtualizedRegistry";

HRESULT PrepareDevirtualizedRegistry::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(ExtractRegistry(false));
    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::ExtractRegistry(bool remove)
{
    std::wstring registryFilePath = m_msixRequest->GetPackageDirectoryPath() + registryDatFile;

    AutoPtr<RegistryDevirtualizer> registryDevirtualizer;
    RETURN_IF_FAILED(RegistryDevirtualizer::Create(registryFilePath, m_msixRequest, &registryDevirtualizer));
    RETURN_IF_FAILED(registryDevirtualizer->Run(remove));
    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::ExecuteForRemoveRequest()
{
    HRESULT hrRemoveRegistry = ExtractRegistry(true);
    if (FAILED(hrRemoveRegistry))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to remove registry",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrRemoveRegistry, "HR"));
    }

    HRESULT hrRemoveVfsFiles = RemoveVfsFiles();
    if (FAILED(hrRemoveVfsFiles))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to remove VFS files",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrRemoveVfsFiles, "HR"));
    }

    // First release manifest so we can delete the file.
    m_msixRequest->GetPackageInfo()->ReleaseManifest();

    std::error_code error;
    auto packageDirectoryPath = m_msixRequest->GetPackageDirectoryPath();
    uintmax_t numRemoved = std::experimental::filesystem::remove_all(packageDirectoryPath, error);

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Removed directory",
        TraceLoggingValue(packageDirectoryPath.c_str(), "PackageDirectoryPath"),
        TraceLoggingValue(error.value(), "Error"),
        TraceLoggingValue(numRemoved, "NumRemoved"));

    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::ConvertVfsNameToFullPath(std::wstring fileName, std::wstring& fileFullPath)
{
    //The following code gets remainingFilePath from "VFS\FirstDir\...\file.ext" to "\...\file.ext"
    std::wstring remainingFilePath = fileName;
    MsixCoreLib_GetPathChild(remainingFilePath); // remove the VFS directory

    std::map<std::wstring, std::wstring> map = FilePathMappings::GetInstance().GetMap();
    for (auto& pair : map)
    {
        if (remainingFilePath.find(pair.first) != std::wstring::npos)
        {
            MsixCoreLib_GetPathChild(remainingFilePath); // remove the FirstDir directory.

            // Pre-pend the VFS target directory to obtain the full path for the target location
            fileFullPath = pair.second + std::wstring(L"\\") + remainingFilePath;

            //Stop looping through the list
            return S_OK;
        }
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Could not find VFS mapping",
        TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
        TraceLoggingValue(fileName.c_str(), "FileName"));

    return E_NOT_SET;
}

HRESULT PrepareDevirtualizedRegistry::RemoveVfsFile(std::wstring fileName)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "RemoveVfsFile",
        TraceLoggingValue(fileName.c_str(), "FileName"));

    std::wstring fullPath;
    if (FAILED(ConvertVfsNameToFullPath(fileName, fullPath)))
    {
        return S_OK;
    }

    if (!DeleteFile(fullPath.c_str()))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to Delete file",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(fullPath.c_str(), "FullPath"),
            TraceLoggingValue(GetLastError(), "error"));
    }

    MsixCoreLib_GetPathParent(fullPath);

    // instead of checking if the directory is empty, just try to delete it.
    // if it's not empty it'll fail with expected error code that we can ignore
    if (!RemoveDirectory(fullPath.c_str()))
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

    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::RemoveVfsFiles()
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

HRESULT PrepareDevirtualizedRegistry::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PrepareDevirtualizedRegistry > localInstance(new PrepareDevirtualizedRegistry(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}