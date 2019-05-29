//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <type_traits>

#include "IXml.hpp"
#include "UnicodeConversion.hpp"

// must remain in same order as XmlAttributeName
static const char* attributeNames[] = {
    /* Name                                   */"Name",
    /* ResourceId                             */"ResourceId",
    /* Version                                */"Version",
    /* Size                                   */"Size",
    /* Package_Identity_ProcessorArchitecture */"ProcessorArchitecture",
    /* Publisher                              */"Publisher",
    /* BlockMap_File_LocalFileHeaderSize      */"LfhSize",
    /* BlockMap_File_Block_Hash               */"Hash",
    /* Bundle_Package_FileName                */"FileName",
    /* Bundle_Package_Offset                  */"Offset",
    /* Bundle_Package_Type                    */"Type",
    /* Bundle_Package_Architecture            */"Architecture",
    /* Language                               */"Language",
    /* MinVersion                             */"MinVersion",
    /* Dependencies_Tdf_MaxVersionTested      */"MaxVersionTested",
    /* Scale                                  */"Scale",
    /* Category                               */"Category",
};

#ifdef USING_MSXML

// must remain in same order as XmlQueryName
static const MSIX::XmlQueryNameCharType* xPaths[] = {
    /* Package_Identity                              */L"/*[local-name()='Package']/*[local-name()='Identity']",
    /* BlockMap_File                                 */L"/*[local-name()='BlockMap']/*[local-name()='File']",
    /* BlockMap_File_Block                           */L"*[local-name()='Block']",
    /* Bundle_Identity                               */L"/*[local-name()='Bundle']/*[local-name()='Identity']",
    /* Bundle_Packages_Package                       */L"/*[local-name()='Bundle']/*[local-name()='Packages']/*[local-name()='Package']",
    /* Bundle_Packages_Package_Resources_Resource    */L"*[local-name()='Resources']/*[local-name()='Resource']",
    /* Package_Dependencies_TargetDeviceFamily       */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='TargetDeviceFamily']",
    /* Package_Applications_Application              */L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']",
    /* Package_Properties                            */L"/*[local-name()='Package']/*[local-name()='Properties']",
    /* Package_Properties_Description                */L"*[local-name()='Description']",
    /* Package_Properties_DisplayName                */L"*[local-name()='DisplayName']",
    /* Package_Properties_PublisherDisplayName       */L"*[local-name()='PublisherDisplayName']",
    /* Package_Properties_Logo                       */L"*[local-name()='Logo']",
    /* Package_Properties_Framework                  */L"*[local-name()='Framework']",
    /* Package_Properties_ResourcePackage            */L"*[local-name()='ResourcePackage']",
    /* Package_Properties_AllowExecution             */L"*[local-name()='AllowExecution']",
    /* Package_Dependencies_PackageDependency        */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='PackageDependency']",
    /* Package_Capabilities_Capability               */L"/*[local-name()='Package']/*[local-name()='Capabilities']/*[local-name()='Capability']",
    /* Package_Resources_Resource                    */L"/*[local-name()='Package']/*[local-name()='Resources']/*[local-name()='Resource']",
    /* Any_Identity                                  */L"*[local-name()='Identity']",
    /* Package_Dependencies_MainPackageDependency    */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='MainPackageDependency']",
    /* Applications_Application_Extensions_Extension */L"*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']",
};
#else

// must remain in same order as XmlQueryName
static const MSIX::XmlQueryNameCharType* xPaths[] = {
    /* Package_Identity                              */"/Package/Identity",
    /* BlockMap_File                                 */"/BlockMap/File",
    /* BlockMap_File_Block                           */"./Block",
    /* Bundle_Identity                               */"/Bundle/Identity",
    /* Bundle_Packages_Package                       */"/Bundle/Packages/Package",
    /* Bundle_Packages_Package_Resources_Resource    */"./Resources/Resource",
    /* Package_Dependencies_TargetDeviceFamily       */"/Package/Dependencies/TargetDeviceFamily",
    /* Package_Applications_Application              */"/Package/Applications/Application",
    /* Package_Properties                            */"/Package/Properties",
    /* Package_Properties_Description                */"./Description",
    /* Package_Properties_DisplayName                */"./DisplayName",
    /* Package_Properties_PublisherDisplayName       */"./PublisherDisplayName",
    /* Package_Properties_Logo                       */"./Logo",
    /* Package_Properties_Framework                  */"./Framework",
    /* Package_Properties_ResourcePackage            */"./ResourcePackage",
    /* Package_Properties_AllowExecution             */"./AllowExecution",
    /* Package_Dependencies_PackageDependency        */"/Package/Dependencies/PackageDependency",
    /* Package_Capabilities_Capability               */"/Package/Capabilities/Capability",
    /* Package_Resources_Resource                    */"/Package/Resources/Resource",
    /* Any_Identity                                  */"./Identity",
    /* Package_Dependencies_MainPackageDependency    */"/Package/Dependencies/MainPackageDependency",
    /* Applications_Application_Extensions_Extension */"./Applications/Application/Extensions/Extension",
};
#endif

namespace MSIX {

    const XmlQueryNameCharType* GetQueryString(XmlQueryName query)
    {
        return xPaths[static_cast<std::underlying_type_t<XmlQueryName>>(query)];
    }

    std::wstring GetAttributeNameString(XmlAttributeName attr)
    {
        return utf8_to_wstring(GetAttributeNameStringUtf8(attr));
    }

    const char* GetAttributeNameStringUtf8(XmlAttributeName attr)
    {
        return attributeNames[static_cast<std::underlying_type_t<XmlAttributeName>>(attr)];
    }

}
