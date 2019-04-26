#pragma once
#include <map>
#include <string>
#include "GeneralUtil.hpp"
namespace MsixCoreLib
{
/// FilePath Mappings maps the VFS tokens to the actual folder on disk.
/// For instance, VFS token: Windows, actual folder: C:\windows
class FilePathMappings
{
public:
    static FilePathMappings& GetInstance();

    std::map < std::wstring, std::wstring > GetMap() { return m_map; }

    std::wstring GetMsixCoreDirectory() { return m_msixCoreDirectory; }

    /// Gets the resolved full path to the executable.
    /// The executable path could be the location of the exe within the package's MsixCore directory
    /// or it could point to a VFS location. 
    /// @param packageExecutablePath - The path to the executable listed in the package's manifest
    ///                                 This is relative to the package's MsixCore directory.
    /// @param packageFullName   - The packageFullName
    /// @return the resolved full path to the executable
    std::wstring GetExecutablePath(std::wstring packageExecutablePath, PCWSTR packageFullName);
    HRESULT GetInitializationResult() { return m_initializationResult; }
private:
    // Disallow creating an instance of this object
    HRESULT InitializePaths();
    FilePathMappings()
    {
        m_initializationResult = InitializePaths();
    }
private:
    std::map < std::wstring, std::wstring > m_map;
    std::wstring m_msixCoreDirectory;
    bool m_isInitialized = false;
    HRESULT m_initializationResult = E_NOT_SET;
};
}
/// Removes the first directory from a path.
/// @param path - A path that contains at least one parent directory
void MsixCoreLib_GetPathChild(std::wstring &path);

/// Removes the innermost child file from a path
/// @param path - A file path 
void MsixCoreLib_GetPathParent(std::wstring &path);
