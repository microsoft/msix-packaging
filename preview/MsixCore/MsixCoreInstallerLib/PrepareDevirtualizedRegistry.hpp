#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    class PrepareDevirtualizedRegistry : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        /// Removes all the files, directories and registry keys written during the add.
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~PrepareDevirtualizedRegistry() {}
    private:
        MsixRequest * m_msixRequest = nullptr;

        PrepareDevirtualizedRegistry() {}
        PrepareDevirtualizedRegistry(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        /// Extracts or removes the registry information contained inside Registry.dat
        ///
        /// @param remove - if true, removes registry information. if false, adds registry information
        HRESULT ExtractRegistry(bool remove);

        /// Resolves the VFS file to the real location
        ///
        /// @param fileName - the VFS file name
        /// @param fileFullPath - the real location full path
        /// @return E_NOT_SET if the VFS name cannot be found in the mapping.
        HRESULT ConvertVfsNameToFullPath(std::wstring fileName, std::wstring &fileFullPath);

        /// Removes a VFS file from the resolved location
        /// This needs to first resolve the VFS file to the real location and then delete it
        /// It also removes the folder if this is the last file in that folder
        /// 
        /// @param fileName - the VFS file name
        HRESULT RemoveVfsFile(std::wstring fileName);

        /// Removes all VFS files in the package
        HRESULT RemoveVfsFiles();
    };
}