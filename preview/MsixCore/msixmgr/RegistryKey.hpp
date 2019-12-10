#pragma once

#include "../GeneralUtil.hpp"
#include "../MsixTraceLoggingProvider.hpp"
namespace MsixCoreLib
{
/// Encapsulates and lifetimes the HKEY used by the RegOpenKey et al APIs.
class RegistryKey
{
public:
    RegistryKey() = default;

    RegistryKey(HKEY hkey) : m_hkey(hkey) {}

    inline ~RegistryKey()
    {
        Close();
    }

    inline void Close()
    {
        if ((m_hkey != nullptr) && (m_hkey != INVALID_HANDLE_VALUE))
        {
            RegCloseKey(m_hkey);
        }
        m_hkey = nullptr;
        m_path.clear();
    }

    inline HKEY Detach()
    {
        HKEY old = m_hkey;
        m_hkey = nullptr;
        m_path.clear();
        return old;
    }

    inline std::wstring GetPath() { return m_path; }
    inline void SetPath(std::wstring path)
    {
        m_path = path;
    }

    inline RegistryKey* operator&()
    {
        return this;
    }

    inline HKEY* AddressOfHkey()
    {
        return &m_hkey;
    }

    inline operator const HKEY() const
    {
        return m_hkey;
    }

    inline operator HKEY()
    {
        return m_hkey;
    }

    HRESULT Open(
        _In_ const HKEY hkey,
        _In_opt_ PCWSTR subkey,
        _In_ const REGSAM sam);

    HRESULT OpenSubKey(
        _In_opt_ PCWSTR subkey,
        _Inout_ RegistryKey * regkey);

    HRESULT OpenSubKey(
        _In_opt_ PCWSTR subkey,
        _In_ const REGSAM sam,
        _Inout_ RegistryKey * regkey);
  
    HRESULT CreateSubKey(
        _In_opt_ PCWSTR subkey,
        _In_ const REGSAM sam,
        _Inout_ RegistryKey * regkey);

    HRESULT GetInfo(
        _Out_opt_ DWORD * subKeysCount,
        _Out_opt_ DWORD * maxSubKeyLength,
        _Out_opt_ DWORD * valuesCount,
        _Out_opt_ DWORD * maxValueNameLength,
        _Out_opt_ DWORD * maxValueSize,
        _Out_opt_ DWORD * securityDescriptorSize,
        _Out_opt_ FILETIME * lastModified);

    HRESULT GetValuesInfo(
        _Out_ DWORD * count,
        _Out_opt_ DWORD * maxNameLength = nullptr,
        _Out_opt_ DWORD * maxValueSize = nullptr);

    HRESULT GetStringValue(
        _In_ PCWSTR name,
        _Out_ std::wstring& value);

    HRESULT EnumValue(
        _In_ const DWORD index,
        _Out_writes_to_(*valueNameLength, *valueNameLength) PWSTR valueName,
        _Out_ DWORD * valueNameLength,
        _Out_ DWORD * valueType,
        _Out_writes_to_(*dataLength, *dataLength) BYTE * data,
        _Out_ DWORD * dataLength);

    HRESULT SetValue(
        _In_opt_ PCWSTR name,
        _In_reads_bytes_opt_(valueSize) const void * value,
        _In_ const DWORD valueSize,
        _In_ const DWORD type);

    HRESULT SetStringValue(
        _In_opt_ PCWSTR name,
        _In_opt_ const std::wstring& value);

    HRESULT DeleteValue(
        _In_opt_ PCWSTR name);

    HRESULT DeleteSubKey(
        _In_ PCWSTR subkey);

    HRESULT DeleteTree(
        _In_ PCWSTR subkey);

    HRESULT KeyExists(
        _In_ PCWSTR subkey,
        _Out_ bool& exists);

    HRESULT SetUInt32Value(
        _In_opt_ PCWSTR name,
        _In_ UINT32 value);

    HRESULT GetUInt32ValueIfExists(
        _In_ PCWSTR name,
        _Out_ UINT32 & value,
        _Out_ bool & exists);

    template<typename TAction> static HRESULT EnumKeyAndDoActionForAllSubkeys(
        _In_ RegistryKey* registryKey,
        _In_ TAction subkeyActionFunction)
    {
        DWORD subKeySize = MAX_PATH + 1;
        WCHAR subKey[MAX_PATH + 1] = L"";

        DWORD index = 0;
        LONG errorCode = RegEnumKeyEx(*registryKey, index, subKey, &subKeySize, NULL, NULL, NULL, NULL);
        while (errorCode == ERROR_SUCCESS)
        {
            bool shouldStopEnumerating = false;
            RETURN_IF_FAILED(subkeyActionFunction(subKey, registryKey, &shouldStopEnumerating));
            if (shouldStopEnumerating)
            {
                break;
            }
            index++;
            subKeySize = MAX_PATH + 1;
            errorCode = RegEnumKeyEx(*registryKey, index, subKey, &subKeySize, NULL, NULL, NULL, NULL);
        }
        if (errorCode != ERROR_NO_MORE_ITEMS)
        {
            RETURN_IF_FAILED(HRESULT_FROM_WIN32(errorCode));
        }

        return S_OK;
    }

protected:
    std::wstring m_path;
    HKEY m_hkey = nullptr;
};
}