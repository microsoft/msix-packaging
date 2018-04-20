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
    ContentTypeXml  = 0,
    AppxBlockMapXml = 1,
    AppxManifestXml = 2,
    AppxBundleManifestXml =3,
};

// defines queries for use in IXmlDom->ForEachElementIn
enum class XmlQueryName : std::uint8_t
{
    Package_Identity                           = 0,
    BlockMap_File                              = 1,
    BlockMap_File_Block                        = 2,
    Bundle_Identity                            = 3,
    Bundle_Packages_Package                    = 4,
    Bundle_Packages_Package_Resources_Resource = 5,
    Package_Dependencies_TargetDeviceFamily    = 6,
};

// defines attribute names for use in IXmlElement-> [GetAttributeValue|GetBase64DecodedAttributeValue]
enum class XmlAttributeName : std::uint8_t
{
    Name                                       = 0,
    ResourceId                                 = 1,
    Version                                    = 2,
    Size                                       = 3,

    Identity_ProcessorArchitecture             = 4,
    Identity_Publisher                         = 5,

    BlockMap_File_LocalFileHeaderSize          = 6,
    BlockMap_File_Block_Hash                   = 7,

    Bundle_Package_FileName                    = 8,
    Bundle_Package_Offset                      = 9,
    Bundle_Package_Type                        = 10,
    Bundle_Package_Architecture                = 11,
    Bundle_Package_Resources_Resource_Language = 12,
};

EXTERN_C const IID IID_IXmlElement;
EXTERN_C const IID IID_IXmlDom;
EXTERN_C const IID IID_IXmlFactory;

#ifndef WIN32
// {ac94449e-442d-4bed-8fca-83770c0f7ee9}
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
};

struct XmlVisitor
{        
    typedef bool(*lambda)(void*, const MSIX::ComPtr<IXmlElement>& );

    void*   context;
    lambda  Callback;

    XmlVisitor(void* c, lambda f) : context(c), Callback(f) {}
};

#ifndef WIN32
// {0e7a446e-baf7-44c1-b38a-216bfa18a1a8}
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
};

#ifndef WIN32
// {f82a60ec-fbfc-4cb9-bc04-1a0fe2b4d5be}
interface IXmlFactory : public IUnknown
#else
class IXmlFactory : public IUnknown
#endif
// An internal interface for creating an IXmlDom object as well as managing XML services lifetime
{
public:
    virtual MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const MSIX::ComPtr<IStream>& stream) = 0;
};

SpecializeUuidOfImpl(IXmlElement);
SpecializeUuidOfImpl(IXmlDom);
SpecializeUuidOfImpl(IXmlFactory);

namespace MSIX {
    MSIX::ComPtr<IXmlFactory> CreateXmlFactory(IMSIXFactory* factory);

    template <class T>
    static T GetNumber(const ComPtr<IXmlElement>& element, XmlAttributeName attribute, T defaultValue)
    {
        const auto& attributeValue = element->GetAttributeValue(attribute);
        bool hasValue = !attributeValue.empty();
        T value = defaultValue;
        if (hasValue) { value = static_cast<T>(std::stoul(attributeValue)); }
        return value;        
    }
}
