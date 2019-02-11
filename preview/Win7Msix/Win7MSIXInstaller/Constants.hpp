#pragma once
#include <string>

static const std::wstring uninstallKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
static const std::wstring uninstallKeySubPath = L"Microsoft\\Windows\\CurrentVersion\\Uninstall"; // this subpath could be under Software or Software\Wow6432Node
static const std::wstring registryDatFile = L"\\registry.dat";
static const std::wstring blockMapFile = L"\\AppxBlockMap.xml";
static const std::wstring manifestFile = L"\\AppxManifest.xml";