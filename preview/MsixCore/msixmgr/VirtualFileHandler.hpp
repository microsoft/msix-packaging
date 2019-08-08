#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    /// VirtualFileHandler handles the files in the VFS directory--on install determining whether it needs to copy the files to the de-virtualized location
    /// and updating the SharedDLL count in the registry for any shared files that may require it.
    /// Sequencing requirements: this assumes the registry.dat is already mounted, and obviously that all the files to have been extracted.
    class VirtualFileHandler : IPackageHandler
    {
    public:
        /// Copies over the VFS files from the package root directory to the actual file system location if necessary
        HRESULT ExecuteForAddRequest();

        /// Removes the VFS files that were copied over, if they're no longer referenced.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~VirtualFileHandler() {}
    private:
        MsixRequest* m_msixRequest = nullptr;

        VirtualFileHandler() {}
        VirtualFileHandler(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// Copies a VFS file from the package root to its resolved location.
        /// for example VFS\ProgramFilesx86\Notepadplusplus\notepadplusplus.exe would get copied over 
        /// to c:\program files (x86)\Notepadplusplus\notepadplusplus.exe 
        /// The mapping between the VFS path and the resolved location is obtained through FilePathMappings::GetMap
        ///
        /// @param nameStr - A full path of the file in the VFS directory
        HRESULT CopyVfsFileToLocal(std::wstring sourceFullPath);

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

        /// Removes a VFS file from the resolved location
        /// This needs to first resolve the VFS file to the real location and then delete it
        /// It also removes the folder if this is the last file in that folder
        /// 
        /// @param fileName - the VFS file name
        HRESULT RemoveVfsFile(std::wstring fileName);

        /// Resolves the VFS file (relative to the package root) to the real location
        ///
        /// @param fileName - the VFS file name relative to the package root, i.e. VFS\FirstDir\...\File.ext
        /// @param targetFullPath - the real location full path
        /// @return E_NOT_SET if the VFS name cannot be found in the mapping.
        HRESULT ConvertVfsNameToFullPath(std::wstring sourceFullPath, std::wstring &targetFullPath);

        /// Resolves the full file path of a VFS file to the real location
        ///
        /// @param sourceFullPath - the full path of the VFS file, i.e. C:\Program Files\MsixCoreApps\<package>\VFS\FirstDir\...\File.ext
        /// @param targetFullPath - the real location full path
        /// @return E_NOT_SET if the VFS name cannot be found in the mapping.
        HRESULT ConvertVfsFullPathToFullPath(std::wstring sourceFullPath, std::wstring & targetFullPath);

        /// Helper used by both ConvertVfsNameToFullPath and ConvertVfsFullPathToFullPath to convert the normalized remainingFilePath
        /// into the real location. 
        ///
        /// @param remainingFilePath - the remainder of the path inside the VFS directory, i.e. FirstDir\...\File.ext
        /// @param targetFullPath - the real location full path
        /// @return E_NOT_SET if the VFS name cannot be found in the mapping.
        HRESULT ConvertRemainingPathToFullPath(std::wstring& remainingFilePath, std::wstring& targetFullPath);

        /// Removes all VFS files in the package
        HRESULT RemoveVfsFiles();
    };
}
