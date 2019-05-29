//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "MSIXFactory.hpp"

// XML file content types/schemas
enum class XmlContentType : std::uint8_t
{
    ContentTypeXml,
    AppxBlockMapXml,
    AppxManifestXml,
    AppxBundleManifestXml,
};

// defines queries for use in IXmlDom::ForEachElementIn
enum class XmlQueryName : std::uint8_t
{
    Package_Identity                           = 0,
    BlockMap_File                              = 1,
    BlockMap_File_Block                        = 2,
    Bundle_Identity                            = 3,
    Bundle_Packages_Package                    = 4,
    Bundle_Packages_Package_Resources_Resource = 5,
    Package_Dependencies_TargetDeviceFamily    = 6,
    Package_Applications_Application           = 7,
    Package_Properties                         = 8,
    Package_Properties_Description             = 9,
    Package_Properties_DisplayName             = 10,
    Package_Properties_PublisherDisplayName    = 11,
    Package_Properties_Logo                    = 12,
    Package_Properties_Framework               = 13,
    Package_Properties_ResourcePackage         = 14,
    Package_Properties_AllowExecution          = 15,
    Package_Dependencies_PackageDependency     = 16,
    Package_Capabilities_Capability            = 17,
    Package_Resources_Resource                 = 18,
    Any_Identity,
    Package_Dependencies_MainPackageDependency,
    Applications_Application_Extensions_Extension,
};

// defines attribute names for use in IXmlElement:: [GetAttributeValue|GetBase64DecodedAttributeValue]
enum class XmlAttributeName : std::uint8_t
{
    Name                                       = 0,
    ResourceId                                 = 1,
    Version                                    = 2,
    Size                                       = 3,
    Identity_ProcessorArchitecture             = 4,
    Publisher                                  = 5,
    BlockMap_File_LocalFileHeaderSize          = 6,
    BlockMap_File_Block_Hash                   = 7,
    Bundle_Package_FileName                    = 8,
    Bundle_Package_Offset                      = 9,
    Bundle_Package_Type                        = 10,
    Bundle_Package_Architecture                = 11,
    Language                                   = 12,
    MinVersion                                 = 13,
    Dependencies_Tdf_MaxVersionTested          = 14,
    Scale                                      = 15,
    Package_Applications_Application_Id        = 16,
    Category,
};

// {ac94449e-442d-4bed-8fca-83770c0f7ee9}
#ifndef WIN32
interface IXmlElement : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IXmlElement : public IUnknown
#endif
// An internal interface for XML elements
{
public:
    virtual std::string               GetAttributeValue(XmlAttributeName attribute) = 0;
    virtual std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) = 0;
    virtual std::string               GetText() = 0;
};
MSIX_INTERFACE(IXmlElement, 0xac94449e,0x442d,0x4bed,0x8f,0xca,0x83,0x77,0x0c,0x0f,0x7e,0xe9);

struct XmlVisitor
{        
    typedef bool(*lambda)(void*, const MSIX::ComPtr<IXmlElement>& );

    void*   context;
    lambda  Callback;

    // Allow for const data to be passed in, but strip the const.
    XmlVisitor(const void* c, lambda f) : context(const_cast<void*>(c)), Callback(f) {}
};

// {0e7a446e-baf7-44c1-b38a-216bfa18a1a8}
#ifndef WIN32
interface IXmlDom : public IUnknown
#else
class IXmlDom : public IUnknown
#endif
// An internal interface for XML document object model
{
public:
    virtual MSIX::ComPtr<IXmlElement> GetDocument() = 0;
    virtual bool ForEachElementIn(
        const MSIX::ComPtr<IXmlElement>& root,
        XmlQueryName query,
        XmlVisitor&  visitor
    ) = 0;

    bool ForEachElementIn(XmlQueryName query, XmlVisitor& visitor)
    {
        return ForEachElementIn(GetDocument(), query, visitor);
    }
};
MSIX_INTERFACE(IXmlDom, 0x0e7a446e,0xbaf7,0x44c1,0xb3,0x8a,0x21,0x6b,0xfa,0x18,0xa1,0xa8);

// {f82a60ec-fbfc-4cb9-bc04-1a0fe2b4d5be}
#ifndef WIN32
interface IXmlFactory : public IUnknown
#else
class IXmlFactory : public IUnknown
#endif
// An internal interface for creating an IXmlDom object as well as managing XML services lifetime
{
public:
    virtual MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const MSIX::ComPtr<IStream>& stream) = 0;
};
MSIX_INTERFACE(IXmlFactory, 0xf82a60ec,0xfbfc,0x4cb9,0xbc,0x04,0x1a,0x0f,0xe2,0xb4,0xd5,0xbe);

namespace MSIX {
    MSIX::ComPtr<IXmlFactory> CreateXmlFactory(IMsixFactory* factory);

    template <class T>
    static T GetNumber(const ComPtr<IXmlElement>& element, XmlAttributeName attribute, T defaultValue)
    {
        const auto& attributeValue = element->GetAttributeValue(attribute);
        bool hasValue = !attributeValue.empty();
        T value = defaultValue;
        if (hasValue) { value = static_cast<T>(std::stoul(attributeValue)); }
        return value;
    }

#ifdef USING_MSXML
    using XmlQueryNameCharType = wchar_t;
#else
    using XmlQueryNameCharType = char;
#endif

    const XmlQueryNameCharType* GetQueryString(XmlQueryName query);

    std::wstring GetAttributeNameString(XmlAttributeName attr);
    const char* GetAttributeNameStringUtf8(XmlAttributeName attr);
}
