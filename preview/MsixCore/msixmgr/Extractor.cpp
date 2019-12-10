#include <windows.h>
#include <iostream>
#include <filesystem> // Microsoft-specific implementation header file name

#include "Extractor.hpp"
#include "../GeneralUtil.hpp"
#include "FootprintFiles.hpp"
#include "FilePaths.hpp"
#include "Constants.hpp"

#include "RegistryDevirtualizer.hpp"
#include <TraceLoggingProvider.h>
#include "../MsixTraceLoggingProvider.hpp"
using namespace MsixCoreLib;

const PCWSTR Extractor::HandlerName = L"Extractor";

HRESULT Extractor::GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream)
{
    std::wstring fullFileName = path + std::wstring(L"\\") + fileName;
    std::wstring longFileName = std::wstring(L"\\\\?\\") + fullFileName;
    RETURN_IF_FAILED(HRESULT_FROM_WIN32(mkdirp(longFileName)));
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(longFileName.c_str(), false, stream));
    return S_OK;
}

HRESULT Extractor::ExtractFile(IAppxFile* file)
{
    Text<WCHAR> fileName;
    RETURN_IF_FAILED(file->GetName(&fileName));

    UINT64 fileSize = 0;
    RETURN_IF_FAILED(file->GetSize(&fileSize));

    ULARGE_INTEGER fileSizeLargeInteger = { 0 };
    fileSizeLargeInteger.QuadPart = fileSize;

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "ExtractFile",
        TraceLoggingValue(fileName.Get(), "FileName"),
        TraceLoggingValue(fileSize, "FileSize"));

    ComPtr<IStream> fileStream;
    RETURN_IF_FAILED(file->GetStream(&fileStream));
    ComPtr<IStream> outputStream;

    auto packageDirectoryPath = m_msixRequest->GetPackageDirectoryPath();

    RETURN_IF_FAILED(GetOutputStream(packageDirectoryPath.c_str(), fileName.Get(), &outputStream));
    RETURN_IF_FAILED(fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr));
    return S_OK;
}

HRESULT Extractor::ExtractFootprintFiles()
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Extracting footprint files from the package");

    auto packageToInstall = std::dynamic_pointer_cast<Package>(m_msixRequest->GetPackageInfo());
    
    for (int i = 0; i < FootprintFilesCount; i++)
    {
        if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
        {
            return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }

        if (packageToInstall != nullptr)
        {
            ComPtr<IAppxFile> footprintFile;
            HRESULT hr = packageToInstall->GetPackageReader()->GetFootprintFile(g_footprintFilesType[i].fileType, &footprintFile);
            if (SUCCEEDED(hr) && footprintFile.Get())
            {
                RETURN_IF_FAILED(ExtractFile(footprintFile.Get()));
            }
            else if (g_footprintFilesType[i].isRequired)
            {
                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "Missing required Footprintfile",
                    TraceLoggingValue(g_footprintFilesType[i].description, "File Description"));
                return hr;
            }
        }
    }
    return S_OK;
}

HRESULT Extractor::ExtractPayloadFiles()
{
    ComPtr<IAppxFilesEnumerator> files;
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Extracting payload files from the package");

    auto packageToInstall = std::dynamic_pointer_cast<Package>(m_msixRequest->GetPackageInfo());
    if (packageToInstall == nullptr)
    {
        return E_FAIL;
    }

    RETURN_IF_FAILED(packageToInstall->GetPackageReader()->GetPayloadFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));

    unsigned int totalNumberFiles = m_msixRequest->GetPackageInfo()->GetNumberOfPayloadFiles();
    unsigned int nbrFilesExtracted = 0;
    while (hasCurrent)
    {
        if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
        {
            return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }
        ComPtr<IAppxFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));

        RETURN_IF_FAILED(ExtractFile(file.Get()));

        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
        ++nbrFilesExtracted;

        float progress = 100.0f * nbrFilesExtracted / totalNumberFiles;
        m_msixRequest->GetMsixResponse()->Update(InstallationStep::InstallationStepExtraction, progress);
    }

    return S_OK;
}

HRESULT Extractor::CreatePackageRoot()
{
    std::wstring packagePath = FilePathMappings::GetInstance().GetMsixCoreDirectory();
    if (!CreateDirectoryW(packagePath.c_str(), nullptr))
    {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_ALREADY_EXISTS)
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(lastError));
        }
    }

    if (!CreateDirectoryW((FilePathMappings::GetInstance().GetMsixCoreDirectory() + m_msixRequest->GetPackageInfo()->GetPackageFullName()).c_str(), nullptr))
    {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_ALREADY_EXISTS)
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(lastError));
        }
    }

    return S_OK;
}

HRESULT Extractor::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(CreatePackageRoot());

    RETURN_IF_FAILED(ExtractPackage());

    return S_OK;
}

HRESULT Extractor::ExecuteForRemoveRequest()
{
    // First release manifest so we can delete the file.
    m_msixRequest->GetPackageInfo()->ReleaseManifest();

    std::error_code error;
    auto packageDirectoryPath = m_msixRequest->GetPackageDirectoryPath();
    uintmax_t numRemoved = std::filesystem::remove_all(packageDirectoryPath, error);

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Removed directory",
        TraceLoggingValue(packageDirectoryPath.c_str(), "PackageDirectoryPath"),
        TraceLoggingValue(error.value(), "Error"),
        TraceLoggingValue(numRemoved, "NumRemoved"));

    return S_OK;
}

HRESULT Extractor::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<Extractor> localInstance(new Extractor(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}

HRESULT Extractor::ExtractPackage()
{
    RETURN_IF_FAILED(ExtractFootprintFiles());
    RETURN_IF_FAILED(ExtractPayloadFiles());
    return S_OK;
}
