#pragma once
#include "GeneralUtil.hpp"
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

class PackageInfo
{
private:
    ComPtr<IAppxManifestReader> m_manifestReader;
    std::wstring m_packageFullName;
    std::wstring m_packageDirectoryPath;
    std::wstring m_executableFilePath;
    std::wstring m_displayName;
    std::wstring m_appUserModelId;
    std::wstring m_applicationId;
    UINT64 m_version = 0;
    Text<WCHAR> m_publisher;

    /// PackageReader and payloadFiles are available on Add, but not Remove because it's created off the original package itself which is no longer available once it's been installed.
    ComPtr<IAppxPackageReader> m_packageReader;
    DWORD m_numberOfPayloadFiles = 0;

    /// Sets the manifest reader, and other fields derived from the manifest
    /// Specifically, packageFullName, packageDirectoryPath, executableFilePath, displayname, version and publisher.
    ///
    /// @param manifestReader - manifestReader to set
    /// @param msix7DirectoryPath - the root msix7 directory path, which is the parent directory of the package directory.
    HRESULT SetManifestReader(IAppxManifestReader* manifestReader, std::wstring msix7DirectoryPath);

    /// Sets the executable path by reading it from the manifest element
    HRESULT SetExecutableAndAppIdFromManifestElement(IMsixElement * element);

    /// Sets the display name by reading it from the manifest element
    HRESULT SetDisplayNameFromManifestElement(IMsixElement * element);

    /// Sets the application model user id from the manifest reader
    HRESULT SetApplicationUserModelIdFromManifestElement(IAppxManifestReader* manifestReader);

public:
    /// Create a PackageInfo using the manifest reader and directory path. This is intended for Remove scenarios where
    /// the actual .msix package file is no longer accessible.
    static HRESULT MakeFromManifestReader(IAppxManifestReader* manifestReader, std::wstring msix7DirectoryPath, PackageInfo** packageInfo);

    /// Create a PackageInfo using the package reader. This is intended for Add scenarios where
    /// the actual .msix package file is given.
    static HRESULT MakeFromPackageReader(IAppxPackageReader* packageReader, std::wstring msix7DirectoryPath, PackageInfo** packageInfo);

    /// When made from manifest reader, it won't have PackageReader available. 
    bool HasPackageReader() { return (m_packageReader.Get() != nullptr); };
    IAppxPackageReader * GetPackageReader() { return m_packageReader.Get(); }
    DWORD GetNumberOfPayloadFiles() { return m_numberOfPayloadFiles; }

    // Getters
    IAppxManifestReader * GetManifestReader() { return m_manifestReader.Get(); }
    std::wstring GetPackageFullName() { return m_packageFullName; }
    std::wstring GetPackageDirectoryPath() { return m_packageDirectoryPath; }
    std::wstring GetExecutableFilePath() { return m_executableFilePath; }
    std::wstring GetDisplayName() { return m_displayName;  }
    std::wstring GetAppModelUserId() { return m_appUserModelId; }
    UINT64 GetVersion() { return m_version; }
    PCWSTR GetPublisher() { return m_publisher.Get(); }

    /// This is meant only to be called when deleting the manifest file; the reader needs to first be released so it can be deleted
    void ReleaseManifest() { m_manifestReader.Release(); }
};


