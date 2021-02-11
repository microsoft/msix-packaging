#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Extractor extracts the filesfrom the package to the file system.
class Extractor : IPackageHandler
{
public:
    /// Extracts the package to the file system.
    /// Creates the package's root directory in c:\program files\msixcoreapps\<package>
    /// Extracts the files from the package to the package's root directory
    HRESULT ExecuteForAddRequest();

    /// Removes all the files, directories written during the add.
    HRESULT ExecuteForRemoveRequest();
    
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
};
}
