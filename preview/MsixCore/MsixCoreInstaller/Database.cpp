#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "RegistryKey.hpp"
#include "Database.hpp"
#include "inc/IPackage.hpp"
#include "PopulatePackageInfo.hpp"

using namespace MsixCoreLib;

const PCWSTR DatabaseKeyPath = L"Software\\Microsoft\\MsixCore\\Packages";

HRESULT Database::AddPackageForUser(PCWSTR user, PCWSTR packageFullName)
{
    RegistryKey hklmKey;
    RETURN_IF_FAILED(hklmKey.Open(HKEY_LOCAL_MACHINE, nullptr, KEY_READ | KEY_WRITE));

    RegistryKey databaseKey;
    RETURN_IF_FAILED(hklmKey.CreateSubKey(DatabaseKeyPath, KEY_READ | KEY_WRITE, &databaseKey));

    RegistryKey userSidKey;
    RETURN_IF_FAILED(databaseKey.CreateSubKey(user, KEY_READ | KEY_WRITE, &userSidKey));

    RegistryKey packageKey;
    RETURN_IF_FAILED(userSidKey.CreateSubKey(packageFullName, KEY_READ | KEY_WRITE, &packageKey));

    return S_OK;
}

HRESULT Database::AddPackageForCurrentUser(PCWSTR packageFullName)
{
    std::wstring userSidString;
    RETURN_IF_FAILED(GetCurrentUserSidString(userSidString));

    RETURN_IF_FAILED(AddPackageForUser(userSidString.c_str(), packageFullName));
    return S_OK;
}

HRESULT Database::RemovePackageForUser(PCWSTR user, PCWSTR packageFullName)
{
    RegistryKey hklmKey;
    RETURN_IF_FAILED(hklmKey.Open(HKEY_LOCAL_MACHINE, nullptr, KEY_READ | KEY_WRITE));

    RegistryKey databaseKey;
    RETURN_IF_FAILED(hklmKey.OpenSubKey(DatabaseKeyPath, KEY_READ | KEY_WRITE, &databaseKey));

    RegistryKey userSidKey;
    RETURN_IF_FAILED(databaseKey.OpenSubKey(user, KEY_READ | KEY_WRITE, &userSidKey));
    RETURN_IF_FAILED(userSidKey.DeleteSubKey(packageFullName));

    return S_OK;
}

HRESULT Database::RemovePackageForCurrentUser(PCWSTR packageFullName)
{
    std::wstring userSidString;
    RETURN_IF_FAILED(GetCurrentUserSidString(userSidString));

    RETURN_IF_FAILED(RemovePackageForUser(userSidString.c_str(), packageFullName));
    return S_OK;
}

HRESULT Database::FindPackagesForCurrentUser(std::vector<std::wstring> & installedPackages)
{
    std::wstring userSidString;
    RETURN_IF_FAILED(GetCurrentUserSidString(userSidString));

    RegistryKey hklmKey;
    RETURN_IF_FAILED(hklmKey.Open(HKEY_LOCAL_MACHINE, nullptr, KEY_READ | KEY_WRITE));

    RegistryKey databaseKey;
    HRESULT hr = hklmKey.OpenSubKey(DatabaseKeyPath, KEY_READ | KEY_WRITE, &databaseKey);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            // No database key yet--this is created the first time anybody installs, so it's ok not to exist.
            return S_OK;
        }
        RETURN_IF_FAILED(hr);
    }

    RegistryKey userSidKey;
    hr = databaseKey.OpenSubKey(userSidString.c_str(), KEY_READ | KEY_WRITE, &userSidKey);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            // No userSid key yet-- this is created the first time this user installs, so it's ok not to exist.
            return S_OK;
        }
        RETURN_IF_FAILED(hr);
    }

    RETURN_IF_FAILED(RegistryKey::EnumKeyAndDoActionForAllSubkeys(&userSidKey,
        [&](PCWSTR subKeyName, RegistryKey*, bool*) -> HRESULT
    {
        installedPackages.push_back(subKeyName);

        return S_OK;
    }));
    
    return S_OK;
}

HRESULT Database::IsInstalledForAnyOtherUser(PCWSTR packageFullName, bool& isInstalled)
{
    isInstalled = false;

    std::wstring userSidString;
    RETURN_IF_FAILED(GetCurrentUserSidString(userSidString));

    RegistryKey hklmKey;
    RETURN_IF_FAILED(hklmKey.Open(HKEY_LOCAL_MACHINE, nullptr, KEY_READ));

    RegistryKey databaseKey;
    RETURN_IF_FAILED(hklmKey.OpenSubKey(DatabaseKeyPath, KEY_READ, &databaseKey));

    RETURN_IF_FAILED(RegistryKey::EnumKeyAndDoActionForAllSubkeys(&databaseKey,
        [&](PCWSTR userSidKeyName, RegistryKey*, bool* foundPackageForOtherUser) -> HRESULT
    {
        if (CaseInsensitiveEquals(userSidKeyName, userSidString))
        {
            /// Ignore the current user. We are looking for other users that have the package
            return S_OK;
        }
        
        RegistryKey userKey;
        RETURN_IF_FAILED(databaseKey.OpenSubKey(userSidKeyName, KEY_READ, &userKey));

        RETURN_IF_FAILED(userKey.KeyExists(packageFullName, isInstalled));
        
        *foundPackageForOtherUser = isInstalled;
        if (isInstalled)
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Found user with package installed",
                TraceLoggingValue(userSidKeyName, "UserSid"),
                TraceLoggingValue(packageFullName, "Package"));
        }
        return S_OK;
    }));

    return S_OK;
}
