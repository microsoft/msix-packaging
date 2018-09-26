#include <windows.h>
#include <tchar.h>
#include <winreg.h>
#include "RegUtil.h"
#include "Logger.h"
#include "Uninstall.h"
#include <algorithm>
#include <string>
#include "GeneralUtil.h"

using namespace std;

//
// Returns the byte length of a wstring
//
// Parameters:
//   str
//      The wstring to get the length of.
//
DWORD RegWStringLength(wstring str)
{
	return static_cast<DWORD>((str.size() + 1) * sizeof(wchar_t));
}

DWORD RegWCharLength(wchar_t* str)
{
	return static_cast<DWORD>((wcslen(str) + 1) * sizeof(wchar_t));
}

std::wstring GetHiveName(HKEY hkey)
{
	if (hkey == HKEY_LOCAL_MACHINE)
	{
		return L"HKEY_LOCAL_MACHINE";
	}
	else if (hkey == HKEY_CURRENT_USER)
	{
		return L"HKEY_CURRENT_USER";
	}
	else
	{
		WriteToLog("Error", "Hive not found.");
		return L"";
	}
}

LSTATUS WriteSZHelper(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName, LPCWSTR data, DWORD dataLength)
{
	HKEY outKey;
	LSTATUS result = RegCreateKeyEx(hkey, subkey, 0, nullptr, 0, KEY_WRITE, nullptr, &outKey, nullptr);
	WriteToLog("RegCreateKeyEx", to_string(result).c_str());
	if (ERROR_SUCCESS == result)
	{
		result = RegSetKeyValue(hkey, subkey, valueName, REG_SZ, data, dataLength);
		WriteToLog("RegSetKeyValue", to_string(result).c_str());
	}
	RegCloseKey(outKey);
	return result;
}

LSTATUS WriteNONEHelper(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName)
{
	HKEY outKey;
	LSTATUS result = RegCreateKeyEx(hkey, subkey, 0, nullptr, 0, KEY_WRITE, nullptr, &outKey, nullptr);
	WriteToLog("RegCreateKeyEx", to_string(result).c_str());
	if (ERROR_SUCCESS == result)
	{
		result = RegSetKeyValue(hkey, subkey, valueName, REG_NONE, new byte[0]{}, 0);
		WriteToLog("RegSetKeyValue", to_string(result).c_str());
	}
	RegCloseKey(outKey);
	return result;
}

// Writes a REG_SZ type registry key.
LSTATUS RegWriteKeyValueSZ(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName, std::wstring* data, TrackerXML* xml)
{
	if (xml != nullptr)
	{
		xml->AddRegKeyToRecord(hkey, subkey);
	}
	return WriteSZHelper(hkey, subkey, valueName, (*data).c_str(), RegWStringLength(*data));
}

LSTATUS RegWriteKeyValueSZ(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName, wchar_t* data, TrackerXML* xml)
{
	if (xml != nullptr)
	{
		xml->AddRegKeyToRecord(hkey, subkey);
	}
	return WriteSZHelper(hkey, subkey, valueName, (LPCWSTR)data, RegWCharLength(data));
}

LSTATUS RegDeleteKeyAndValues(std::wstring fullKeyPath, bool isx64)
{
	size_t hiveDivide = fullKeyPath.find(L"\\");
	std::wstring hiveName = fullKeyPath.substr(0, hiveDivide);
	HKEY hive = nullptr;
	if (hiveName == L"HKEY_LOCAL_MACHINE")
	{
		hive = HKEY_LOCAL_MACHINE;
	}
	else if (hiveName == L"HKEY_CURRENT_USER")
	{
		hive = HKEY_CURRENT_USER;
	}
	else
	{
		WriteToLog("Error", "Unsupported hive");
		return ERROR_FUNCTION_FAILED;
	}

	std::wstring subkey = fullKeyPath.substr(hiveDivide + 1, fullKeyPath.length() - hiveDivide);
	if (isx64)
	{
		return RegDeleteKeyExW(hive, subkey.c_str(), KEY_WOW64_64KEY, 0);
	}
	else
	{
		return RegDeleteKeyExW(hive, subkey.c_str(), KEY_WOW64_32KEY, 0);
	}
}

LSTATUS RegDeleteAll(TrackerXML* xml)
{
	RegistryEnumerator regEnum = xml->GetRegistryEnumerator();
	while (regEnum.HasNext())
	{
		bool isx64;
		std::wstring fullKey = regEnum.GetCurrent(&isx64);
		RegDeleteKeyAndValues(fullKey, isx64);
		regEnum.MoveNext();
	}

	bool isx64;
	std::wstring fullKey = regEnum.GetCurrent(&isx64);
	return RegDeleteKeyAndValues(fullKey, isx64);
}

LSTATUS RegAddFTA(std::string extensionName, std::wstring exePath, TrackerXML* xml)
{
	std::string progId = extensionName.substr(1, extensionName.length() - 2);
	std::transform(progId.begin(), progId.end(), progId.begin(), ::toupper);
	std::wstring wProgId = utf8_to_utf16(progId);
	std::wstring wExtensionName = utf8_to_utf16(extensionName);

	//Put program into Open With menu
	std::wstring explorerPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + wExtensionName + L"\\OpenWithProgids\\";
	if (xml != nullptr)
	{
		xml->AddRegKeyToRecord(HKEY_CURRENT_USER, explorerPath.c_str());
	}
	WriteNONEHelper(HKEY_CURRENT_USER, explorerPath.c_str(), wProgId.c_str());

	std::wstring extensionPath = L"SOFTWARE\\Classes\\" + wExtensionName + L"\\" + wProgId + L"\\ShellNew\\";
	HKEY outKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, extensionPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &outKey, nullptr);
	RegCloseKey(outKey);

	std::wstring progIdPath = L"SOFTWARE\\Classes\\" + wProgId;
	std::wstring shellPath = progIdPath + L"\\shell\\";
	std::wstring shellDefault = L"open";
	RegWriteKeyValueSZ(HKEY_CURRENT_USER, shellPath.c_str(), _T(""), &shellDefault, xml);

	std::wstring openPath = shellPath + L"open\\";
	std::wstring openDefault = L"&Open";
	RegWriteKeyValueSZ(HKEY_CURRENT_USER, openPath.c_str(), _T(""), &openDefault, xml);

	std::wstring commandPath = openPath + L"command\\";
	std::wstring commandDefault = exePath + L" %1";
	return RegWriteKeyValueSZ(HKEY_CURRENT_USER, commandPath.c_str(), _T(""), &commandDefault, xml);
}

LSTATUS RegAddProtocol(std::wstring protocol, std::wstring exePath, TrackerXML* xml)
{
	std::wstring subkey = L"SOFTWARE\\Classes\\" + protocol + L"\\shell\\open\\command\\";
	std::wstring commandDefault = exePath + L" %1";
	return RegWriteKeyValueSZ(HKEY_CURRENT_USER, subkey.c_str(), _T(""), &commandDefault, xml);
}
