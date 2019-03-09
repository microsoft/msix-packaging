#pragma once
#include <string>

static const std::wstring uninstallKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
static const std::wstring uninstallKeySubPath = L"Microsoft\\Windows\\CurrentVersion\\Uninstall"; // this subpath could be under Software or Software\Wow6432Node
static const std::wstring registryDatFile = L"\\registry.dat";
static const std::wstring blockMapFile = L"\\AppxBlockMap.xml";
static const std::wstring manifestFile = L"\\AppxManifest.xml";
static const std::wstring classesKeyPath = L"SOFTWARE\\Classes";

static const std::wstring msix7ProgIDPrefix = L"Msix7";
static const std::wstring openWithProgIdsKeyName = L"OpenWithProgids";
static const std::wstring shellKeyName = L"Shell";
static const std::wstring openKeyName = L"open";
static const std::wstring commandKeyName = L"command";
static const std::wstring defaultIconKeyName = L"DefaultIcon";
static const std::wstring protocolValueName = L"URL Protocol";
static const std::wstring commandArgument = L" \"%1\"";

static const std::wstring ftaCategoryNameInManifest = L"windows.fileTypeAssociation";
static const std::wstring protocolCategoryNameInManifest = L"windows.protocol";

static const std::wstring categoryAttribute = L"Category";
static const std::wstring nameAttribute = L"Name";
static const std::wstring parametersAttribute = L"Parameters";

static const std::wstring extensionQuery = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']";
static const std::wstring ftaQuery = L"*[local-name()='FileTypeAssociation']";
static const std::wstring fileTypeQuery = L"*[local-name()='SupportedFileTypes']/*[local-name()='FileType']";
static const std::wstring logoQuery = L"*[local-name()='Logo']";
static const std::wstring verbQuery = L"*[local-name()='SupportedVerbs']/*[local-name()='Verb']";
static const std::wstring protocolQuery = L"*[local-name()='Protocol']";
static const std::wstring displayNameQuery = L"*[local-name()='DisplayName']";