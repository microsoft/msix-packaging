#pragma once
#include <windows.h>
#include <string>
#include "Uninstall.h"

DWORD RegWCharLength(wchar_t* str);

DWORD RegWCharLength(wchar_t* str);

std::wstring GetHiveName(HKEY hkey);

LSTATUS RegWriteKeyValueSZ(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName, std::wstring* data, TrackerXML* xml = nullptr);

LSTATUS RegWriteKeyValueSZ(HKEY hkey, LPCWSTR subkey, LPCWSTR valueName, wchar_t* data, TrackerXML* xml = nullptr);

LSTATUS RegDeleteKeyAndValues(std::wstring fullKeyPath, bool is64);

LSTATUS RegDeleteAll(TrackerXML* xml);

LSTATUS RegAddFTA(std::string extensionName, std::wstring exePath, TrackerXML* xml = nullptr);

LSTATUS RegAddProtocol(std::wstring protocol, std::wstring exePath, TrackerXML* xml = nullptr);
