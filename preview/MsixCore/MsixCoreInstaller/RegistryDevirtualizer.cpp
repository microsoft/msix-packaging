#include "RegistryDevirtualizer.hpp"
#include "FilePaths.hpp"
#include "Constants.hpp"
#include <vector>
#include "MsixTraceLoggingProvider.hpp"

using namespace MsixCoreLib;

struct VirtualRegistryMapping
{
    PCWSTR virtualKey;
    HKEY actualRootHive;
    PCWSTR actualSubkey;
};

VirtualRegistryMapping mappings[] =
{
    //virtualKey from registry.dat,             root hive,              Subkey off of root hive
    {L"USER\\.DEFAULT",                         HKEY_USERS,             L".DEFAULT"},
    {L"USER\\[{AppVCurrentUserSID}]_CLASSES",   HKEY_CURRENT_USER,      L"Software\\Classes"},
    {L"USER\\[{AppVCurrentUserSID}]",           HKEY_CURRENT_USER,      L""},
    {L"MACHINE",                                HKEY_LOCAL_MACHINE,     L""},
};

HRESULT RegistryDevirtualizer::Run(_In_ bool remove)
{
    if (!m_hiveFileNameExists)
    {
        return S_OK;
    }

    std::wstring rootPath = m_loadedHiveKeyName + L"\\Registry";
    RETURN_IF_FAILED(m_rootKey.Open(HKEY_USERS, rootPath.c_str(), KEY_READ));
    
    for (auto mapping : mappings)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Processing mapping",
            TraceLoggingValue(mapping.virtualKey, "Virtual Key"));

        RegistryKey virtualKey;
        HRESULT hrOpenVirtualKey = m_rootKey.OpenSubKey(mapping.virtualKey, KEY_READ, &virtualKey);
        if (hrOpenVirtualKey == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Skipping mapping because it does not exist",
                TraceLoggingValue(mapping.virtualKey, "Virtual Key"));
            continue;
        }
        else
        {
            RETURN_IF_FAILED(hrOpenVirtualKey);
        }

        RegistryKey realKey;
        RETURN_IF_FAILED(realKey.Open(mapping.actualRootHive, mapping.actualSubkey, KEY_READ | KEY_WRITE | WRITE_DAC));

        if (remove)
        {
            RETURN_IF_FAILED(RemoveDevirtualizeRegistryTree(&virtualKey, &realKey));
        }
        else
        {
            RETURN_IF_FAILED(CopyAndDevirtualizeRegistryTree(&virtualKey, &realKey));
        }
    }

    return S_OK;
}

HRESULT RegistryDevirtualizer::HasFTA(std::wstring ftaName, bool & hasFTA)
{
    hasFTA = false;
    if (!m_hiveFileNameExists)
    {
        return S_OK;
    }
    std::wstring rootPath = m_loadedHiveKeyName + L"\\Registry";
    RETURN_IF_FAILED(m_rootKey.Open(HKEY_USERS, rootPath.c_str(), KEY_READ));

    RegistryKey userClassesKey;
    HRESULT hrOpenUserClassesKey = m_rootKey.OpenSubKey(L"USER\\[{AppVCurrentUserSID}]_CLASSES", KEY_READ, &userClassesKey);
    if (SUCCEEDED(hrOpenUserClassesKey))
    {
        RegistryKey ftaKey;
        HRESULT hrFtaKey = userClassesKey.OpenSubKey(ftaName.c_str(), KEY_READ, &ftaKey);
        if (SUCCEEDED(hrFtaKey))
        {
            hasFTA = true;
            return S_OK;
        }
    }

    RegistryKey machineClassesKey;
    HRESULT hrOpenMachineClassesKey = m_rootKey.OpenSubKey(L"MACHINE\\Software\\Classes", KEY_READ, &machineClassesKey);
    if (SUCCEEDED(hrOpenMachineClassesKey))
    {
        RegistryKey ftaKey;
        HRESULT hrFtaKey = machineClassesKey.OpenSubKey(ftaName.c_str(), KEY_READ, &ftaKey);
        if (SUCCEEDED(hrFtaKey))
        {
            hasFTA = true;
            return S_OK;
        }
    }

    return S_OK;
}

bool RegistryDevirtualizer::IsExcludeKey(RegistryKey* realKey)
{
    const std::wstring excludeKeys[] = 
    {
        uninstallKeySubPath,   // uninstall key will be written by MsixInstaller itself, no need to copy from package
    };

    for (auto excludeKey : excludeKeys)
    {
        if (realKey->GetPath().find(excludeKey) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

HRESULT RegistryDevirtualizer::CopyAndDevirtualizeRegistryTree(RegistryKey* virtualKey, RegistryKey* realKey)
{
    if (IsExcludeKey(realKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Key is in exclusion list: not creating anything further",
            TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"));
        return S_OK;
    }

    RETURN_IF_FAILED(RegistryKey::EnumKeyAndDoActionForAllSubkeys(virtualKey,
        [&](PCWSTR subKeyName, RegistryKey*, bool*) -> HRESULT
    {
        RegistryKey sourceSubKey;
        RETURN_IF_FAILED(virtualKey->OpenSubKey(subKeyName, KEY_READ, &sourceSubKey));

        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Creating subkey",
            TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
            TraceLoggingValue(subKeyName, "SubKey"));

        RegistryKey destinationSubKey;
        HRESULT hrCreateKey = HRESULT_FROM_WIN32(realKey->CreateSubKey(subKeyName, KEY_READ | KEY_WRITE | WRITE_DAC, &destinationSubKey));
        if (hrCreateKey == HRESULT_FROM_WIN32(ERROR_CHILD_MUST_BE_VOLATILE))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Skipping attempted write to volatile key",
                TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
                TraceLoggingValue(subKeyName, "SubKey"));
            return S_OK;
        }
        else if (FAILED(hrCreateKey))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to Create key",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
                TraceLoggingValue(subKeyName, "SubKey"),
                TraceLoggingValue(hrCreateKey, "HR"));
            return S_OK;
        }

        RETURN_IF_FAILED(CopyAndDevirtualizeRegistryTree(&sourceSubKey, &destinationSubKey));
        return S_OK;
    }));
    
    DWORD valuesCount = 0;
    DWORD valueNameMaxLength = 0;
    DWORD valueDataMaxLength = 0;
    RETURN_IF_FAILED(virtualKey->GetValuesInfo(&valuesCount, &valueNameMaxLength, &valueDataMaxLength));

    if (valuesCount == 0)
    {
        // no values -- nothing more to devirtualize, return early.
        return S_OK;
    }

    DWORD valueType = REG_NONE;
    std::vector<wchar_t> valueNameBuffer(valueNameMaxLength + 1);
    std::vector<BYTE> valueDataBuffer((valueDataMaxLength + 1));

    // for each value of this key, devirtualize it and write it to the real key.
    for (DWORD i = 0; i < valuesCount; i++)
    {
        DWORD nameLength = static_cast<DWORD>(valueNameBuffer.size());
        DWORD dataLength = static_cast<DWORD>(valueDataBuffer.size());
        valueType = REG_NONE;

        RETURN_IF_FAILED(virtualKey->EnumValue(i, &valueNameBuffer[0], &nameLength, &valueType,
            reinterpret_cast<LPBYTE>(&valueDataBuffer[0]), &dataLength));

        std::wstring valueNameString(valueNameBuffer.begin(), valueNameBuffer.end());
        RETURN_IF_FAILED(DevirtualizeValue(realKey, valueNameString.c_str(), valueDataBuffer, dataLength, valueType));
    }
    return S_OK;
}

HRESULT RegistryDevirtualizer::DetokenizeData(std::wstring& data)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Detokenizing string",
        TraceLoggingValue(data.c_str(), "data"));

    std::wstring::size_type beginToken = data.find(L"[{");
    std::wstring::size_type endToken = data.find(L"}]");
    while (beginToken != std::wstring::npos && endToken != std::wstring::npos)
    {
        bool foundToken = false;
        // get the contents of what's in between the braces, i.e. [{token}]
        std::wstring token = data.substr(beginToken + 2, (endToken - beginToken - 2)); // +2 to skip over [{ characters, -2 to omit }] characters

        std::map<std::wstring, std::wstring> map = FilePathMappings::GetInstance().GetMap();
        for (auto& pair : map)
        {
            if (token.find(pair.first) != std::wstring::npos)
            {
                // replace the entire braces [{token}] with what it represents
                data.replace(beginToken, endToken + 2 - beginToken, pair.second); // +2 to include the }] characters
                foundToken = true;
                break;
            }
        }

        if (!foundToken)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Fail to find token",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(token.c_str(), "token"));
            break;
        }

        beginToken = data.find(L"[{");
        endToken = data.find(L"}]");
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Detokenizing result:",
        TraceLoggingValue(data.c_str(), "data"));

    return S_OK;
}

HRESULT RegistryDevirtualizer::DevirtualizeValue(RegistryKey* realKey, PCWSTR name, std::vector<BYTE>& dataBuffer, DWORD dataLength, DWORD type)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Devirtualizing value info",
        TraceLoggingValue(name, "value name"),
        TraceLoggingValue(type, "valueType"));

    DWORD actualType = (type & 0xff);
    if (actualType == REG_SZ)
    {
        std::wstring dataWString = reinterpret_cast<PWSTR>(&dataBuffer[0]);
        RETURN_IF_FAILED(DetokenizeData(dataWString));
        RETURN_IF_FAILED(realKey->SetStringValue(name, dataWString));
    }
    else
    {
        RETURN_IF_FAILED(realKey->SetValue(name, reinterpret_cast<LPBYTE>(&dataBuffer[0]), dataLength, actualType));
    }

    return S_OK;
}

HRESULT RemoveSubKeyIfEmpty(RegistryKey* realKey, PCWSTR subKeyName)
{
    // if there are no values left, try to delete the subkey
    RegistryKey subKey;
    RETURN_IF_FAILED(realKey->OpenSubKey(subKeyName, KEY_READ | KEY_WRITE | WRITE_DAC, &subKey));

    DWORD valuesCount = 0;
    DWORD valueNameMaxLength = 0;
    DWORD valueDataMaxLength = 0;
    RETURN_IF_FAILED(subKey.GetValuesInfo(&valuesCount, &valueNameMaxLength, &valueDataMaxLength));

    subKey.Close();
    if (valuesCount == 0)
    {
        HRESULT hrDeleteKey = realKey->DeleteSubKey(subKeyName);
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Attempt to delete subkey",
            TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
            TraceLoggingValue(subKeyName, "Subkey"),
            TraceLoggingValue(hrDeleteKey, "HR"));
    }
    else
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Key not empty -- not deleting subkey",
            TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
            TraceLoggingValue(subKeyName, "Subkey"),
            TraceLoggingValue(valuesCount, "Remaining value count"));
    }

    return S_OK;
}

HRESULT RegistryDevirtualizer::RemoveDevirtualizeRegistryTree(RegistryKey * virtualKey, RegistryKey * realKey)
{
    if (IsExcludeKey(realKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Key is in exclusion list: not processing further",
            TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"));
        return S_OK;
    }

    RETURN_IF_FAILED(RegistryKey::EnumKeyAndDoActionForAllSubkeys(virtualKey,
        [&](PCWSTR subKeyName, RegistryKey*, bool*) -> HRESULT
    {
        RegistryKey sourceSubKey;
        RETURN_IF_FAILED(virtualKey->OpenSubKey(subKeyName, KEY_READ, &sourceSubKey));

        RegistryKey destinationSubKey;
        HRESULT hrCreateKey = HRESULT_FROM_WIN32(realKey->OpenSubKey(subKeyName, KEY_READ | KEY_WRITE | WRITE_DAC, &destinationSubKey));
        if (hrCreateKey == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Key not found -- nothing to delete",
                TraceLoggingValue(realKey->GetPath().c_str(), "Realkey"),
                TraceLoggingValue(subKeyName, "SubKey"));
            return S_OK;
        }
        RETURN_IF_FAILED(hrCreateKey);

        RETURN_IF_FAILED(RemoveDevirtualizeRegistryTree(&sourceSubKey, &destinationSubKey));

        destinationSubKey.Close();
        RETURN_IF_FAILED(RemoveSubKeyIfEmpty(realKey, subKeyName));
        return S_OK;
    }));

    DWORD valuesCount = 0;
    DWORD valueNameMaxLength = 0;
    DWORD valueDataMaxLength = 0;
    RETURN_IF_FAILED(virtualKey->GetValuesInfo(&valuesCount, &valueNameMaxLength, &valueDataMaxLength));

    if (valuesCount > 0)
    {
        DWORD valueType = REG_NONE;
        std::vector<wchar_t> valueNameBuffer(valueNameMaxLength + 1);
        std::vector<BYTE> valueDataBuffer((valueDataMaxLength + 1));

        // for each value of this key, devirtualize it and delete it from the real key.
        for (DWORD i = 0; i < valuesCount; i++)
        {
            DWORD nameLength = static_cast<DWORD>(valueNameBuffer.size());
            DWORD dataLength = static_cast<DWORD>(valueDataBuffer.size());
            valueType = REG_NONE;

            RETURN_IF_FAILED(virtualKey->EnumValue(i, &valueNameBuffer[0], &nameLength, &valueType,
                reinterpret_cast<LPBYTE>(&valueDataBuffer[0]), &dataLength));

            std::wstring valueNameString(valueNameBuffer.begin(), valueNameBuffer.end());
            RETURN_IF_FAILED(realKey->DeleteValue(valueNameString.c_str()));
        }
    }

    return S_OK;
}

HRESULT RegistryDevirtualizer::Create(std::wstring hiveFileName, MsixRequest* msixRequest, RegistryDevirtualizer ** instance)
{
    bool registryFileExists = false;
    RETURN_IF_FAILED(FileExists(hiveFileName, registryFileExists));

    std::unique_ptr<RegistryDevirtualizer> localInstance(new RegistryDevirtualizer(hiveFileName, msixRequest));
    if (localInstance == nullptr)
    {
        RETURN_IF_FAILED(E_OUTOFMEMORY);
    }

    if (registryFileExists)
    {
        localInstance->m_hiveFileNameExists = true;
        HANDLE userToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &userToken))
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
        }

        LUID seRestoreLuid;

        if (!LookupPrivilegeValue(
            NULL,            // lookup privilege on local system
            SE_RESTORE_NAME,   // privilege to lookup 
            &seRestoreLuid))        // receives LUID of privilege
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
        }

        LUID seBackupLuid;

        if (!LookupPrivilegeValue(
            NULL,            // lookup privilege on local system
            SE_BACKUP_NAME,   // privilege to lookup 
            &seBackupLuid))        // receives LUID of privilege
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
        }

        PTOKEN_PRIVILEGES pTokenPrivileges = NULL;

        // be sure we allocate enought space for 2 LUID_AND_ATTRIBUTES
        // by default TOKEN_PRIVILEGES allocates space for only 1 LUID_AND_ATTRIBUTES.
        pTokenPrivileges = (PTOKEN_PRIVILEGES)LocalAlloc(LMEM_FIXED, sizeof(TOKEN_PRIVILEGES) + (sizeof(LUID_AND_ATTRIBUTES) * 2));
        pTokenPrivileges->PrivilegeCount = 2;
        pTokenPrivileges->Privileges[0].Luid = seRestoreLuid;
        pTokenPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        pTokenPrivileges->Privileges[1].Luid = seBackupLuid;
        pTokenPrivileges->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

        auto success = AdjustTokenPrivileges(userToken, FALSE, pTokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
        LocalFree(pTokenPrivileges);
        pTokenPrivileges = NULL;
        if (!success)
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
        }

        RETURN_IF_FAILED(CreateTempKeyName(localInstance->m_loadedHiveKeyName));
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(RegLoadKey(HKEY_USERS, localInstance->m_loadedHiveKeyName.c_str(), localInstance->m_registryHiveFileName.c_str())));
    }

    *instance = localInstance.release();
    
    return S_OK;
}

HRESULT RegistryDevirtualizer::CreateTempKeyName(std::wstring &tempName)
{
    GUID guid;
    RETURN_IF_FAILED(CoCreateGuid(&guid));

    WCHAR stringGuid[40] = {};
    StringFromGUID2(guid, stringGuid, ARRAYSIZE(stringGuid));

    tempName = stringGuid;
    return S_OK;
}

RegistryDevirtualizer::~RegistryDevirtualizer()
{
    if (m_hiveFileNameExists)
    {
        m_rootKey.Close();

        LSTATUS status = RegUnLoadKey(HKEY_USERS, m_loadedHiveKeyName.c_str());

        if (status != ERROR_SUCCESS)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Failed to unload key",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(status, "Error LSTATUS"),
                TraceLoggingValue(m_loadedHiveKeyName.c_str(), "Hive key name"));
        }
    }
}
