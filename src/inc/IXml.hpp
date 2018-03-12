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
};

// defines queries for use in IXmlDom->ForEachElementIn
enum class XmlQueryName : std::uint8_t
{
    Package_Identity,
    BlockMap_File,
    BlockMap_File_Block,
};

// defines attribute names for use in IXmlElement-> [GetAttributeValue|GetBase64DecodedAttributeValue]
enum class XmlAttributeName : std::uint8_t
{
    Package_Identity_Name,
    Package_Identity_ProcessorArchitecture,
    Package_Identity_Publisher,
    Package_Identity_Version,
    Package_Identity_ResourceId,

    BlockMap_File_Name,
    BlockMap_File_LocalFileHeaderSize,
    BlockMap_File_Block_Size,
    BlockMap_File_Block_Hash,
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
        IXmlElement* root,
        XmlQueryName query,
        std::function<bool(IXmlElement*)> visitor
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
    virtual MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, MSIX::ComPtr<IStream>& stream) = 0;
};

SpecializeUuidOfImpl(IXmlElement);
SpecializeUuidOfImpl(IXmlDom);
SpecializeUuidOfImpl(IXmlFactory);

namespace MSIX {
    MSIX::ComPtr<IXmlFactory> CreateXmlFactory(IMSIXFactory* factory);
}
