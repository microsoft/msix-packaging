#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Extractor extracts the files and registry settings from the package to the file system and registry.
class Extractor : IPackageHandler
{
public:
    /// Extracts the package to the file system and registry.
    /// Creates the package's root directory in c:\program files\msixcoreapps\<package>
    /// Extracts the files from the package to the package's root directory
    /// Copies over the VFS files from the package root directory to the actual file system location
    /// Devirtualizes the registry keys from the package's registry.dat
    HRESULT ExecuteForAddRequest();
    
    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~Extractor() {}
private:
    MsixRequest* m_msixRequest =  nullptr;

    Extractor() {}
    Extractor(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Extracts all files from a package.
    HRESULT ExtractPackage();

    /// Writes the file from the package to disk.
    ///
    /// @param file - The IAppxFile interface that represents a footprint or payload file 
    ///                in the package.
    HRESULT ExtractFile(IAppxFile* file);

    /// Extracts all footprint files (i.e. manifest/blockmap/signature) from a package.
    HRESULT ExtractFootprintFiles();

    /// Extracts all payload files from a package.
    HRESULT ExtractPayloadFiles();

    /// Creates a writable IStream over a file with the specified name
    /// under the specified path.  This function will also create intermediate
    /// subdirectories if necessary.
    ///
    /// @param path - Path of the folder containing the file to be opened.  This should NOT
    ///            end with a slash ('\') character.
    /// @param fileName - Name, not including path, of the file to be opened
    /// @param stream    - Output parameter pointing to the created instance of IStream over
    ///              the specified file when this function succeeds.
    HRESULT GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream);

    /// Creates the package root directory where all the files will be installed to.
    /// This will be in c:\program files\msixcoreapps\<packagefullname>
    HRESULT CreatePackageRoot();
    
    /// Copies a VFS file from the package root to its resolved location.
    /// for example VFS\ProgramFilesx86\Notepadplusplus\notepadplusplus.exe would get copied over 
    /// to c:\program files (x86)\Notepadplusplus\notepadplusplus.exe 
    /// The mapping between the VFS path and the resolved location is obtained through FilePathMappings::GetMap
    ///
    /// @param nameStr - A filepath of the file in the VFS 
    HRESULT CopyVfsFileToLocal(std::wstring nameStr);

    /// Determines if a file needs to be copied. 
    /// If a file already exists in the target location, the highest version file will be retained
    /// This follows MSI versioning rules.
    ///
    /// @param sourceFullPath - The source file 
    /// @param targetFullPath - The target location to copy to
    /// @param needToCopyFile - whether we need to copy the file
    HRESULT NeedToCopyFile(std::wstring sourceFullPath, std::wstring targetFullPath, bool & needToCopyFile);

    /// Determine whether we need to copy a VFS file from the package root to its resolved location
    ///
    /// @param sourceFullPath - The source file 
    /// @param targetFullPath - The target location to copy to
    HRESULT CopyVfsFileIfNecessary(std::wstring sourceFullPath, std::wstring targetFullPath);

    /// Resolves the VFS file to the real location
    ///
    /// @param fileName - the VFS file name
    /// @param fileFullPath - the real location full path
    /// @return E_NOT_SET if the VFS name cannot be found in the mapping.
    HRESULT ConvertVfsNameToFullPath(std::wstring fileName, std::wstring &fileFullPath);
};
}
