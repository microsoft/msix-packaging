#pragma once
#include <string>

static const std::wstring uninstallKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
static const std::wstring uninstallKeySubPath = L"Microsoft\\Windows\\CurrentVersion\\Uninstall"; // this subpath could be under Software or Software\Wow6432Node
static const std::wstring registryDatFile = L"\\registry.dat";
static const std::wstring blockMapFile = L"\\AppxBlockMap.xml";
static const std::wstring manifestFile = L"\\AppxManifest.xml";
static const std::wstring classesKeyPath = L"SOFTWARE\\Classes";
static const std::wstring desktopTargetDeviceFamilyName = L"MSIXCore.Desktop";
static const std::wstring serverTargetDeviceFamilyName = L"MSIXCore.Server";

static const std::wstring msixCoreProgIDPrefix = L"MsixCore";
static const std::wstring openWithProgIdsKeyName = L"OpenWithProgids";
static const std::wstring shellKeyName = L"Shell";
static const std::wstring openKeyName = L"open";
static const std::wstring commandKeyName = L"command";
static const std::wstring defaultIconKeyName = L"DefaultIcon";
static const std::wstring protocolValueName = L"URL Protocol";
static const std::wstring urlProtocolPrefix = L"URL:";
static const std::wstring commandArgument = L" \"%1\"";

static const std::wstring ftaCategoryNameInManifest = L"windows.fileTypeAssociation";
static const std::wstring protocolCategoryNameInManifest = L"windows.protocol";
static const std::wstring ComServerCategoryNameInManifest = L"windows.comServer";
static const std::wstring comInterfaceCategoryNameInManifest = L"windows.comInterface";
static const std::wstring startupTaskCategoryNameInManifest = L"windows.startupTask";

static const std::wstring categoryAttribute = L"Category";
static const std::wstring nameAttribute = L"Name";
static const std::wstring parametersAttribute = L"Parameters";
static const std::wstring idAttribute = L"Id";
static const std::wstring executableAttribute = L"Executable";
static const std::wstring argumentsAttribute = L"Arguments";
static const std::wstring displayNameAttribute = L"DisplayName";
static const std::wstring launchAndActivationPermissionAttribute = L"LaunchAndActivationPermission";
static const std::wstring progIdAttribute = L"ProgId";
static const std::wstring versionIndependentProgIdAttribute = L"VersionIndependentProgId";
static const std::wstring clsidAttribute = L"Clsid";
static const std::wstring currentVersionAttribute = L"CurrentVersion";
static const std::wstring autoConvertToAttribute = L"AutoConvertTo";
static const std::wstring insertableObjectAttribute = L"InsertableObject";
static const std::wstring shortDisplayNameAttribute = L"ShortDisplayName";
static const std::wstring enableOleDefaultHandlerAttribute = L"EnableOleDefaultHandler";
static const std::wstring formatNameAttribute = L"FormatName";
static const std::wstring standardFormatAttribute = L"StandardFormat";
static const std::wstring defaultFormatNameAttribute = L"DefaultFormatName";
static const std::wstring defaultStandardFormatAttribute = L"DefaultStandardFormat";
static const std::wstring aspectFlagAttribute = L"AspectFlag";
static const std::wstring mediumFlagAttribute = L"MediumFlag";
static const std::wstring directionAttribute = L"Direction";
static const std::wstring versionNumberAttribute = L"VersionNumber";
static const std::wstring proxyStubClsidAttribute = L"ProxyStubClsid";
static const std::wstring proxyStubClsidForUniversalMarshaler = L"{00020424-0000-0000-C000-000000000046}";
static const std::wstring localeIdAttribute = L"LocaleId";
static const std::wstring libraryFlagAttribute = L"LibraryFlag";
static const std::wstring helpDirectoryAttribute = L"HelpDirectory";
static const std::wstring pathAttribute = L"Path";
static const std::wstring typeAttribute = L"Type";
static const std::wstring oleMiscFlagAttribute = L"OleMiscFlag";
static const std::wstring appendMenuFlagAttribute = L"AppendMenuFlag";
static const std::wstring oleVerbFlagAttribute = L"OleVerbFlag";
static const std::wstring resourceIndexAttribute = L"ResourceIndex";
static const std::wstring taskIdAttribute = L"TaskId";
static const std::wstring IdAttribute = L"Id";

static const std::wstring extensionQuery = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']";
static const std::wstring ftaQuery = L"*[local-name()='FileTypeAssociation']";
static const std::wstring fileTypeQuery = L"*[local-name()='SupportedFileTypes']/*[local-name()='FileType']";
static const std::wstring logoQuery = L"*[local-name()='Logo']";
static const std::wstring verbQuery = L"*[local-name()='SupportedVerbs']/*[local-name()='Verb']";
static const std::wstring protocolQuery = L"*[local-name()='Protocol']";
static const std::wstring displayNameQuery = L"*[local-name()='DisplayName']";
static const std::wstring exeServerQuery = L"*[local-name()='ComServer']/*[local-name()='ExeServer']";
static const std::wstring exeServerClassQuery = L"*[local-name()='Class']";
static const std::wstring progIdQuery = L"*[local-name()='ComServer']/*[local-name()='ProgId']";
static const std::wstring implementedCategoriesQuery = L"*[local-name()='ImplementedCategories']/*[local-name()='ImplementedCategory']";
static const std::wstring readableFormatsQuery = L"*[local-name()='Conversion']/*[local-name()='Readable']/*[local-name()='Format']";
static const std::wstring readWritableFormatsQuery = L"*[local-name()='Conversion']/*[local-name()='ReadWritable']/*[local-name()='Format']";
static const std::wstring dataFormatsQuery = L"*[local-name()='DataFormats']";
static const std::wstring dataFormatQuery = L"*[local-name()='DataFormat']";
static const std::wstring interfaceQuery = L"*[local-name()='ComInterface']/*[local-name()='Interface']";
static const std::wstring typeLibForInterfaceQuery = L"*[local-name()='TypeLib']";
static const std::wstring typeLibQuery = L"*[local-name()='ComInterface']/*[local-name()='TypeLib']";
static const std::wstring versionQuery = L"*[local-name()='Version']";
static const std::wstring win32PathQuery = L"*[local-name()='Win32Path']";
static const std::wstring win64PathQuery = L"*[local-name()='Win64Path']";
static const std::wstring miscStatusQuery = L"*[local-name()='MiscStatus']";
static const std::wstring aspectQuery = L"*[local-name()='Aspect']";
static const std::wstring defaultIconQuery = L"*[local-name()='DefaultIcon']";
static const std::wstring toolboxBitmapQuery = L"*[local-name()='ToolboxBitmap32']";
static const std::wstring comVerbQuery = L"*[local-name()='Verbs']/*[local-name()='Verb']";
static const std::wstring startupTaskQuery = L"*[local-name()='StartupTask']";
static const std::wstring launchActionQuery = L"*[local-name()='AutoPlayContent']/*[local-name()='LaunchAction']";
static const std::wstring invokeActionQuery = L"*[local-name()='AutoPlayHandler']/*[local-name()='InvokeAction']";
static const std::wstring invokeActionContentQuery = L"*[local-name()='Content']";
static const std::wstring invokeActionDeviceQuery = L"*[local-name()='Device']";

/// Constants for Firewall DEH
static const std::wstring firewallExtensionQuery = L"/*[local-name()='Package']/*[local-name()='Extensions']/*[local-name()='Extension']/*[local-name()='FirewallRules']/*[local-name()='Rule']";
static const std::wstring firewallRuleDirectionAttribute = L"Direction";
static const std::wstring protocolAttribute = L"IPProtocol";
static const std::wstring profileAttribute = L"Profile";
static const std::wstring localPortMinAttribute = L"LocalPortMin";
static const std::wstring localPortMaxAttribute = L"LocalPortMax";
static const std::wstring remotePortMinAttribute = L"RemotePortMin";
static const std::wstring remotePortMaxAttribute = L"RemotePortMax";
static const std::wstring directionIn = L"in";
static const std::wstring directionOut = L"out";

/// Constants for AutoPlay DEH
static const std::wstring desktopAppXExtensionCategory = L"windows.autoPlayHandler";
static const std::wstring idAttributeName = L"Verb";
static const std::wstring actionAttributeName = L"ActionDisplayName";
static const std::wstring providerAttributeName = L"ProviderDisplayName";
static const std::wstring contentEventAttributeName = L"ContentEvent";
static const std::wstring deviceEventAttributeName = L"DeviceEvent";
static const std::wstring dropTargetHandlerAttributeName = L"DropTargetHandler";
static const std::wstring parametersAttributeName = L"Parameters";
static const std::wstring hwEventHandlerAttributeName = L"HWEventHandler";
static const std::wstring InitCmdLineAttributeName = L"InitCmdLine";
static const std::wstring dropTargetRegKeyName = L"DropTarget";
static const std::wstring commandKeyRegName = L"command";
static const wchar_t desktopAppXProtocolDelegateExecuteValue[] = L"{BFEC0C93-0B7D-4F2C-B09C-AFFFC4BDAE78}";

static const std::wstring explorerRegKeyName = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer";
static const std::wstring handlerKeyName = L"AutoPlayHandlers\\Handlers";
static const std::wstring eventHandlerRootRegKeyName = L"AutoPlayHandlers\\EventHandlers";

/// Constants for AppExecutionAlias DEH
static const std::wstring appExecutionAliasCategory = L"windows.appExecutionAlias";
static const std::wstring executionAliasQuery = L"*[local-name()='AppExecutionAlias']/*[local-name()='ExecutionAlias']";
static const std::wstring executionAliasName = L"Alias";
static const std::wstring appPathsRegKeyName = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths";

static const std::wstring clsidKeyName = L"CLSID";
static const std::wstring inprocHandlerKeyName = L"InprocHandler32";
static const std::wstring defaultInprocHandler = L"ole32.dll";
static const std::wstring localServerKeyName = L"LocalServer32";
static const std::wstring progIdKeyName = L"ProgID";
static const std::wstring versionIndependentProgIdKeyName = L"VersionIndependentProgID";
static const std::wstring curVerKeyName = L"CurVer";
static const std::wstring auxUserTypeKeyName = L"AuxUserType";
static const std::wstring shortDisplayNameKeyName = L"2";
static const std::wstring insertableObjectKeyName = L"Insertable";
static const std::wstring autoConvertToValueName = L"AutoConvertTo";
static const std::wstring implementedCategoriesKeyName = L"Implemented Categories";
static const std::wstring readableKeyName = L"Conversion\\Readable\\Main";
static const std::wstring readWritableKeyName = L"Conversion\\ReadWritable\\Main";
static const std::wstring dataFormatsKeyName = L"DataFormats";
static const std::wstring defaultFileKeyName = L"DefaultFile";
static const std::wstring getSetKeyName = L"GetSet";
static const std::wstring interfaceKeyName = L"Interface";
static const std::wstring proxyStubClsidKeyName = L"ProxyStubClsid32";
static const std::wstring typeLibKeyName = L"TypeLib";
static const std::wstring versionValueName = L"Version";
static const std::wstring win64KeyName = L"win64";
static const std::wstring win32KeyName = L"win32";
static const std::wstring flagsKeyName = L"Flags";
static const std::wstring helpDirKeyName = L"HelpDir";
static const std::wstring miscStatusKeyName = L"MiscStatus";
static const std::wstring verbKeyName = L"Verb";
static const std::wstring toolboxBitmapKeyName = L"ToolBoxBitmap32";