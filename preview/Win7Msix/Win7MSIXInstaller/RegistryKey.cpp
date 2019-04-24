#include "RegistryKey.hpp"
using namespace Win7MsixInstallerLib;

HRESULT RegistryKey::Open(
    _In_ const HKEY hkey,
    _In_opt_ PCWSTR subkey,
    _In_ const REGSAM sam)
{
    Close();
    if (subkey != nullptr)
    {
        m_path = subkey;
    }
    return HRESULT_FROM_WIN32(RegOpenKeyExW(hkey, subkey, 0, sam, AddressOfHkey()));
}

HRESULT RegistryKey::OpenSubKey(
    _In_opt_ PCWSTR subkey,
    _Inout_ RegistryKey * regkey)
{
    return OpenSubKey(subkey, 0 /*sam*/, regkey);
}

HRESULT RegistryKey::OpenSubKey(
    _In_opt_ PCWSTR subkey,
    _In_ const REGSAM sam,
    _Inout_ RegistryKey * regkey)
{
    regkey->Close();
    regkey->SetPath(m_path + L"\\" + subkey);
    return HRESULT_FROM_WIN32(RegOpenKeyExW(this->m_hkey, subkey, 0, sam, regkey->AddressOfHkey()));
}

HRESULT RegistryKey::CreateSubKey(PCWSTR subkey, const REGSAM sam, RegistryKey * regkey)
{
    regkey->Close();
    regkey->SetPath(m_path + L"\\" + subkey);
    return HRESULT_FROM_WIN32(RegCreateKeyExW(this->m_hkey, subkey, 0 /*reserved*/, nullptr /*class*/, 0 /* options */, sam,
        nullptr /*securityAttributes*/, regkey->AddressOfHkey(), nullptr /*disposition*/));
}

HRESULT RegistryKey::EnumValue(
    _In_ const DWORD index,
    _Out_writes_to_(*valueNameLength, *valueNameLength) PWSTR valueName,
    _Out_ DWORD * valueNameLength,
    _Out_ DWORD * valueType,
    _Out_writes_to_(*dataLength, *dataLength) BYTE * data,
    _Out_ DWORD * dataLength)
{
    return HRESULT_FROM_WIN32(RegEnumValueW(this->m_hkey, index, valueName, valueNameLength, nullptr/*reserved*/, valueType, data, dataLength));
}

HRESULT RegistryKey::GetInfo(
    _Out_opt_ DWORD * subKeysCount,
    _Out_opt_ DWORD * maxSubKeyLength,
    _Out_opt_ DWORD * valuesCount,
    _Out_opt_ DWORD * maxValueNameLength,
    _Out_opt_ DWORD * maxValueSize,
    _Out_opt_ DWORD * securityDescriptorSize,
    _Out_opt_ FILETIME * lastModified)
{
    return HRESULT_FROM_WIN32(RegQueryInfoKeyW(this->m_hkey, nullptr /*class*/, nullptr/*cchClass*/, nullptr /*reserved*/,
        subKeysCount, maxSubKeyLength,
        nullptr /*maxClass*/,
        valuesCount, maxValueNameLength,
        maxValueSize, securityDescriptorSize,
        lastModified));
}

HRESULT RegistryKey::GetValuesInfo(
    _Out_ DWORD * count,
    _Out_opt_ DWORD * maxNameLength,
    _Out_opt_ DWORD * maxValueSize)
{
    return GetInfo(nullptr/*subkeysCount*/, nullptr /*maxSubKeyLength*/, 
        count, maxNameLength, maxValueSize, 
        nullptr /*securityDescriptorSize*/, nullptr /*lastModified*/);
}

HRESULT RegistryKey::GetStringValue(PCWSTR name, std::wstring & value)
{
    WCHAR buffer[MAX_PATH];
    DWORD bufferSize = ARRAYSIZE(buffer);
    DWORD type = 0;
    RETURN_IF_FAILED(HRESULT_FROM_WIN32(RegQueryValueExW(this->m_hkey, name, nullptr /*reserved*/, &type, reinterpret_cast<BYTE *>(buffer), &bufferSize)));
    if (type != REG_SZ)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    value = buffer;
    return S_OK;
}

HRESULT RegistryKey::SetValue(
    _In_opt_ PCWSTR name,
    _In_reads_bytes_opt_(valueSize) const void * value,
    _In_ const DWORD valueSize,
    _In_ const DWORD type)
{
    return HRESULT_FROM_WIN32(RegSetValueExW(this->m_hkey, name, 0, type, static_cast<const BYTE*>(value), valueSize));
}

HRESULT RegistryKey::SetStringValue(PCWSTR name, const std::wstring& value)
{
    return SetValue(name, value.c_str(), static_cast<DWORD>(value.size() * sizeof(WCHAR)), REG_SZ);
}

HRESULT RegistryKey::DeleteValue(_In_opt_ PCWSTR name)
{
    HRESULT hr =  HRESULT_FROM_WIN32(RegDeleteValueW(this->m_hkey, name));
    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        return S_OK;
    }
    return hr;
}

HRESULT RegistryKey::DeleteSubKey(PCWSTR subkey)
{
    return HRESULT_FROM_WIN32(RegDeleteKeyW(this->m_hkey, subkey));
}

HRESULT RegistryKey::DeleteTree(PCWSTR subkey)
{
    return HRESULT_FROM_WIN32(RegDeleteTreeW(this->m_hkey, subkey));
}
