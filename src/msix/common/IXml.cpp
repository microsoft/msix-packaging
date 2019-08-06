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
    /* Package_Applications_Application_Id    */"Id",
    /* Category                               */"Category",
    /* MaxMajorVersionTested                  */"MaxMajorVersionTested",
};

#ifdef USING_MSXML

// must remain in same order as XmlQueryName
static const MSIX::XmlQueryNameCharType* xPaths[] = {
    /* Package_Identity                              */L"/*[local-name()='Package']/*[local-name()='Identity']",
    /* BlockMap_File                                 */L"/*[local-name()='BlockMap']/*[local-name()='File']",
    /* Child_Block                                   */L"*[local-name()='Block']",
    /* Bundle_Identity                               */L"/*[local-name()='Bundle']/*[local-name()='Identity']",
    /* Bundle_Packages_Package                       */L"/*[local-name()='Bundle']/*[local-name()='Packages']/*[local-name()='Package']",
    /* Child_Resources_Resource                      */L"*[local-name()='Resources']/*[local-name()='Resource']",
    /* Package_Dependencies_TargetDeviceFamily       */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='TargetDeviceFamily']",
    /* Package_Applications_Application              */L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']",
    /* Package_Properties                            */L"/*[local-name()='Package']/*[local-name()='Properties']",
    /* Child_Description                             */L"*[local-name()='Description']",
    /* Child_DisplayName                             */L"*[local-name()='DisplayName']",
    /* Child_PublisherDisplayName                    */L"*[local-name()='PublisherDisplayName']",
    /* Child_Logo                                    */L"*[local-name()='Logo']",
    /* Child_Framework                               */L"*[local-name()='Framework']",
    /* Child_ResourcePackage                         */L"*[local-name()='ResourcePackage']",
    /* Child_AllowExecution                          */L"*[local-name()='AllowExecution']",
    /* Package_Dependencies_PackageDependency        */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='PackageDependency']",
    /* Package_Capabilities_Capability               */L"/*[local-name()='Package']/*[local-name()='Capabilities']/*[local-name()='Capability']",
    /* Package_Resources_Resource                    */L"/*[local-name()='Package']/*[local-name()='Resources']/*[local-name()='Resource']",
    /* Child_Identity                                */L"*[local-name()='Identity']",
    /* Package_Dependencies_MainPackageDependency    */L"/*[local-name()='Package']/*[local-name()='Dependencies']/*[local-name()='MainPackageDependency']",
    /* Package_Applications                          */L"/*[local-name()='Package']/*[local-name()='Applications']",
    /* Package_Capabilities                          */L"/*[local-name()='Package']/*[local-name()='Capabilities']",
    /* Package_Extensions                            */L"/*[local-name()='Package']/*[local-name()='Extensions']",
    /* Package_Properties_Framework                  */L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='Framework']",
    /* Package_Properties_ResourcePackage            */L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='ResourcePackage']",
    /* Package_Properties_SupportedUsers             */L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='SupportedUsers']",
    /* Package_Capabilities_CustomCapability         */L"/*[local-name()='Package']/*[local-name()='Capabilities']/*[local-name()='CustomCapability']",
};
#else

// must remain in same order as XmlQueryName
static const MSIX::XmlQueryNameCharType* xPaths[] = {
    /* Package_Identity                              */"/Package/Identity",
    /* BlockMap_File                                 */"/BlockMap/File",
    /* Child_Block                                   */"./Block",
    /* Bundle_Identity                               */"/Bundle/Identity",
    /* Bundle_Packages_Package                       */"/Bundle/Packages/Package",
    /* Child_Resources_Resource                      */"./Resources/Resource",
    /* Package_Dependencies_TargetDeviceFamily       */"/Package/Dependencies/TargetDeviceFamily",
    /* Package_Applications_Application              */"/Package/Applications/Application",
    /* Package_Properties                            */"/Package/Properties",
    /* Child_Description                             */"./Description",
    /* Child_DisplayName                             */"./DisplayName",
    /* Child_PublisherDisplayName                    */"./PublisherDisplayName",
    /* Child_Logo                                    */"./Logo",
    /* Child_Framework                               */"./Framework",
    /* Child_ResourcePackage                         */"./ResourcePackage",
    /* Child_AllowExecution                          */"./AllowExecution",
    /* Package_Dependencies_PackageDependency        */"/Package/Dependencies/PackageDependency",
    /* Package_Capabilities_Capability               */"/Package/Capabilities/Capability",
    /* Package_Resources_Resource                    */"/Package/Resources/Resource",
    /* Child_Identity                                */"./Identity",
    /* Package_Dependencies_MainPackageDependency    */"/Package/Dependencies/MainPackageDependency",
    /* Package_Applications                          */"/Package/Applications",
    /* Package_Capabilities                          */"/Package/Capabilities",
    /* Package_Extensions                            */"/Package/Extensions",
    /* Package_Properties_Framework                  */"/Package/Properties/Framework",
    /* Package_Properties_ResourcePackage            */"/Package/Properties/ResourcePackage",
    /* Package_Properties_SupportedUsers             */"/Package/Properties/SupportedUsers",
    /* Package_Capabilities_CustomCapability         */"/Package/Capabilities/CustomCapability",
};
#endif

namespace MSIX {

    const XmlQueryNameCharType* GetQueryString(XmlQueryName query)
    {
        return xPaths[static_cast<std::underlying_type_t<XmlQueryName>>(query)];
    }

    std::string GetQueryStringUtf8(XmlQueryName query)
    {
#ifdef USING_MSXML
        return wstring_to_utf8(GetQueryString(query));
#else
        return GetQueryString(query);
#endif
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
