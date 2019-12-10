#pragma once

#include "../GeneralUtil.hpp"
#include "MsixRequest.hpp"
#include "RegistryKey.hpp"
#include <vector>
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Handles the conversion of the information stored in Registry.dat 
/// to the currently running OS's registry, which is referred to here as "actual registry"
/// The information stored in Registry.dat is referred to here as "virtual registry" or "virtualized registry"
class RegistryDevirtualizer
{
public:

    /// Add or remove the entries contained in Registry.dat
    ///
    /// @param remove - if true, remove all the keys and values; if false, add all the keys and values
    HRESULT Run(_In_ bool remove);

    /// Determines whether Registry.dat contains a FTA (file type association). Does not perform any modifications to the system.
    /// 
    /// @param ftaName - the name of the FTA extension (i.e. .mp4)
    HRESULT HasFTA(_In_ std::wstring ftaName, _Out_ bool& hasFTA);

    static HRESULT Create(_In_ std::wstring hiveFileName, _In_ MsixRequest* msixRequest, _Out_ std::shared_ptr<RegistryDevirtualizer>* instance);

    /// Creates a GUID string as a temporary registry key's name
    /// The Registry.dat hive will be loaded under this name to avoid conflict with existing keys or other installs
    ///
    /// @param tempName - string containing the name
    static HRESULT CreateTempKeyName(std::wstring & tempName);

    HRESULT UnloadMountedHive();

    ~RegistryDevirtualizer();

private:
    /// Determines if the registry key in question should not be written to the actual registry.
    /// Certain keys should not be written because they will be written by this installers.
    /// For instance, AddRemovePrograms will write an uninstall key, 
    /// so we should not write the uninstall key from the virtualized registry 
    ///
    /// @param realKey - registry key on the actual OS
    /// @return true if the key should be excluded.
    bool IsExcludeKey(RegistryKey * realKey);

    /// Recursively copy the registry tree from the loaded Registry.dat to the actual registry.
    ///
    /// @param virtualKey - registry key from the loaded Registry.dat to copy from
    /// @param realKey - registry key in the actual registry to write to
    HRESULT CopyAndDevirtualizeRegistryTree(RegistryKey* virtualKey, RegistryKey* realKey);

    /// Replaces the tokens with their real counterparts
    /// For instance, [{ProgramFilesX86}]\notepad\notepad.exe becomes c:\program files (x86)\notepad\notepad.exe
    /// The tokens are based on the FilePathMappings 
    /// 
    /// @param data - the string to detokenize; this is the registry data to be written
    HRESULT DetokenizeData(std::wstring & data);

    /// Writes the value to the actual key after detokenizing it if necessary
    ///
    /// @param realKey - registry key from actual registry to write the value to
    /// @param name - registry value name, can be null for (default)
    /// @param dataBuffer - registry data
    /// @param dataLength - length of registry data
    /// @param type - registry data type, which can have AppV flags.
    HRESULT DevirtualizeValue(RegistryKey * realKey, PCWSTR name, std::vector<BYTE>& dataBuffer, DWORD dataLength, DWORD type);

    /// Recursively removes the data contained in the loaded Registry.dat from the actual registry.
    /// Remove counterpart of @see CopyAndDevirtualizeRegistryTree
    ///
    /// @param virtualKey - registry key from the loaded Registry.dat as a template of what keys to delete
    /// @param realKey - registry key in the actual registry to delete keys from
    HRESULT RemoveDevirtualizeRegistryTree(RegistryKey* virtualKey, RegistryKey* realKey);

private:
    MsixRequest* m_msixRequest = nullptr;

    RegistryDevirtualizer() {}
    RegistryDevirtualizer(_In_ std::wstring hiveFileName, _In_ MsixRequest* msixRequest)
        : m_registryHiveFileName(hiveFileName),
          m_msixRequest(msixRequest),
          m_rootKey(nullptr) {}

    std::wstring m_loadedHiveKeyName;
    std::wstring m_registryHiveFileName;

    RegistryKey m_rootKey;
    bool m_hiveFileNameExists = false;
};
}