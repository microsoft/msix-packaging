//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"
#include "StreamHelper.hpp"
#include "MSIXResource.hpp"
#include "UnicodeConversion.hpp"
#include "Enumerators.hpp"

// Mandatory for using any feature of Xerces.
#include "xercesc/dom/DOM.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/framework/XMLGrammarPoolImpl.hpp"
#include "xercesc/parsers/AbstractDOMParser.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/Base64.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "xercesc/util/XMLEntityResolver.hpp"

XERCES_CPP_NAMESPACE_USE

// An internal interface for XML document object model
// {07d6ee0e-2165-4b90-8024-e176291e77dd}
#ifndef WIN32
interface IXercesElement : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IXercesElement : public IUnknown
#endif
{
public:
    virtual DOMElement* GetElement() = 0;
};
MSIX_INTERFACE(IXercesElement,  0x07d6ee0e,0x2165,0x4b90,0x80,0x24,0xe1,0x76,0x29,0x1e,0x77,0xdd);

namespace MSIX {

static std::map<std::string, std::string> s_nameSpaceToSchema = 
{
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10",
            "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10",
            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/types",
            "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"},
        {"http://schemas.microsoft.com/appx/2014/phone/manifest",
            "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/2",
            "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/2",
            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/3",
            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/4",
            "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities",
            "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/2",
            "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd " },
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/3",
            "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities",
            "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/2",
            "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/3",
            "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4",
            "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/5",
            "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v5.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/6",
            "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v6.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/mobile/windows10",
            "AppxPackaging/Manifest/Schema/2015/MobileManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/iot/windows10",
            "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/iot/windows10/2",
            "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/holographic/windows10",
            "AppxPackaging/Manifest/Schema/2015/HolographicManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/serverpreview/windows10",
            "AppxPackaging/Manifest/Schema/2015/ServerManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10",
            "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10/2",
            "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10/3",
            "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10/4",
            "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10/5",
            "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v5.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/desktop/windows10/6",
            "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v6.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/com/windows10",
            "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/com/windows10/2",
            "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/5",
            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/6",
            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/7",
            "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v7.xsd"},
        {"http://schemas.microsoft.com/appx/manifest/uap/windows10/8",
            "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v8.xsd"},
// Bundle Manifest
        {"http://schemas.microsoft.com/appx/2013/bundle",
            "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd"},
        {"http://schemas.microsoft.com/appx/2016/bundle",
            "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd"},
        {"http://schemas.microsoft.com/appx/2017/bundle",
            "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd"},
        {"http://schemas.microsoft.com/appx/2018/bundle",
            "AppxPackaging/Manifest/Schema/2018/BundleManifestSchema2018.xsd"}
};

class ParsingException final : public XERCES_CPP_NAMESPACE::ErrorHandler
{
public:
    ParsingException() {};
    ~ParsingException() {};

    void warning(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {        
        ThrowErrorAndLog(MSIX::Error::XmlWarning, GetMessage(exp).c_str());
    }

    void error(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {
        ThrowErrorAndLog(MSIX::Error::XmlError, GetMessage(exp).c_str());
    }

    void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {
        ThrowErrorAndLog(MSIX::Error::XmlFatal, GetMessage(exp).c_str());
    }

    void resetErrors() override {}
private:
    std::string GetMessage(const XERCES_CPP_NAMESPACE::SAXParseException& exp)
    {
        std::u16string utf16FileName = std::u16string(exp.getSystemId());
        std::u16string utf16Message = std::u16string(exp.getMessage());
        return "Error " + u16string_to_utf8(utf16Message) + " file: " + u16string_to_utf8(utf16FileName) +
            ":" + std::to_string(static_cast<std::uint64_t>(exp.getLineNumber())) + "," +
            std::to_string(static_cast<std::uint64_t>(exp.getColumnNumber()));
    }
};

class MsixEntityResolver : public XMLEntityResolver
{
public:
    MsixEntityResolver(IMsixFactory* factory) : m_factory(factory) {}
    ~MsixEntityResolver() {}

    InputSource* resolveEntity(XMLResourceIdentifier* resourceIdentifier)
    {
        std::u16string utf16string = std::u16string(resourceIdentifier->getNameSpace());
        std::string id = u16string_to_utf8(utf16string);
        auto xsd = s_nameSpaceToSchema.find(id);
        ThrowErrorIf(MSIX::Error::XmlError, xsd == s_nameSpaceToSchema.end(), "Invalid namespace");
        auto stream = m_factory->GetResource(xsd->second);
        auto schemaBuffer = Helper::CreateRawBufferFromStream(stream);
        auto item = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(schemaBuffer.second), schemaBuffer.first, xsd->second.c_str(), true);
        return item.release();
    }
private:
    IMsixFactory* m_factory = nullptr;
};

template<class T>
class XercesPtr
{
public:
    XercesPtr() : m_ptr(nullptr) {}
    XercesPtr(T* p)  : m_ptr(p) {}

    // move ctor
    XercesPtr(XercesPtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesPtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    } 

    virtual ~XercesPtr() { InternalRelease(); }

    void InternalRelease()
    {
        T* temp = m_ptr;
        if (temp)
        {
            m_ptr = nullptr;
            temp->release();
        }
    }
    
    XercesPtr& operator=(XercesPtr&& right)
    {   XercesPtr(std::move(right)).Swap(*this);
        return *this;
    }

    T* operator->() const { return m_ptr; }
    T* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    T* m_ptr = nullptr;
};

class XercesCharPtr
{
public:
    XercesCharPtr(char* c) : m_ptr(c) {};
    ~XercesCharPtr() { InternalRelease(); }

    // move ctor
    XercesCharPtr(XercesCharPtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesCharPtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    } 

    void InternalRelease()
    {   XERCES_CPP_NAMESPACE::XMLString::release(&m_ptr);
        m_ptr = nullptr;
    }

    XercesCharPtr& operator=(XercesCharPtr&& right)
    {   XercesCharPtr(std::move(right)).Swap(*this);
        return *this;
    }

    char* operator->() const { return m_ptr; }
    char* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesCharPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    char* m_ptr = nullptr;        
};

class XercesXMLChPtr
{
public:
    XercesXMLChPtr(XMLCh* c) : m_ptr(c) {}
    ~XercesXMLChPtr() { InternalRelease(); }

    // move ctor
    XercesXMLChPtr(XercesXMLChPtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesXMLChPtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    }

    void InternalRelease()
    {   XERCES_CPP_NAMESPACE::XMLString::release(&m_ptr);
        m_ptr = nullptr;
    }

    XercesXMLChPtr& operator=(XercesXMLChPtr&& right)
    {   XercesXMLChPtr(std::move(right)).Swap(*this);
        return *this;
    }

    XMLCh* operator->() const { return m_ptr; }
    XMLCh* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesXMLChPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    XMLCh* m_ptr = nullptr;              
};

class XercesXMLBytePtr
{
public:
    XercesXMLBytePtr(XMLByte* c) : m_ptr(c) {}
    ~XercesXMLBytePtr() { InternalRelease(); }

    // move ctor
    XercesXMLBytePtr(XercesXMLBytePtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesXMLBytePtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    }  

    void InternalRelease()
    {   delete(m_ptr);
        m_ptr = nullptr;
    }

    XercesXMLBytePtr& operator=(XercesXMLBytePtr&& right)
    {   XercesXMLBytePtr(std::move(right)).Swap(*this);
        return *this;
    }

    XMLByte* operator->() const { return m_ptr; }
    XMLByte* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesXMLBytePtr& right ) { std::swap(m_ptr, right.m_ptr); }
    XMLByte* m_ptr = nullptr;             
};    

class XercesElement final : public ComClass<XercesElement, IXmlElement, IXercesElement, IMsixElement>
{
private:
    std::string GetAttributeValue(std::string& attributeName)
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(attributeName.c_str()));
        auto utf16string = std::u16string(m_element->getAttribute(nameAttr.Get()));
        return u16string_to_utf8(utf16string);
    }

public:
    XercesElement(IMsixFactory* factory, DOMElement* element, XERCES_CPP_NAMESPACE::XercesDOMParser* parser) :
        m_factory(factory), m_element(element), m_parser(parser)
    {
        m_resolver = XercesPtr<DOMXPathNSResolver>(m_parser->getDocument()->createNSResolver(m_parser->getDocument()));
    }
    
    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        auto attributeName = wstring_to_utf8(attributeNames[static_cast<uint8_t>(attribute)]);
        return GetAttributeValue(attributeName);
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(wstring_to_utf8(attributeNames[static_cast<uint8_t>(attribute)]).c_str()));
        XMLSize_t len = 0;
        XercesXMLBytePtr decodedData(XERCES_CPP_NAMESPACE::Base64::decodeToXMLByte(
            m_element->getAttribute(nameAttr.Get()),
            &len));
        std::vector<std::uint8_t> result(len);
        for(XMLSize_t index=0; index < len; index++)
        {   result[index] = static_cast<std::uint8_t>(decodedData.Get()[index]);
        }
        return result;
    }

    std::string GetText() override
    {
        DOMNode* node = dynamic_cast<DOMNode*>(m_element);
        XercesCharPtr value(XMLString::transcode(node->getTextContent()));
        return std::string(value.Get());
    }

    // IXercesElement
    DOMElement* GetElement() override { return m_element; }

     // IMsixElement
    HRESULT STDMETHODCALLTYPE GetAttributeValue(LPCWSTR name, LPWSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        auto intermediate = wstring_to_utf8(name);
        auto attributeValue = GetAttributeValue(intermediate);
        return m_factory->MarshalOutString(attributeValue, value);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetText(LPWSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        auto text = GetText();
        return m_factory->MarshalOutString(text, value);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetElements(LPCWSTR name, IMsixElementEnumerator** elements) noexcept override try
    {
        return GetElementsUtf8(wstring_to_utf8(name).c_str(), elements);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetAttributeValueUtf8(LPCSTR name, LPSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        auto attribute = std::string(name);
        auto attributeValue = GetAttributeValue(attribute);
        return m_factory->MarshalOutStringUtf8(attributeValue, value);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetTextUtf8(LPSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        auto text = GetText();
        return m_factory->MarshalOutStringUtf8(text, value);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetElementsUtf8(LPCSTR name, IMsixElementEnumerator** elements) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (elements == nullptr || *elements != nullptr), "bad pointer.");
        // Note: getElementsByTagName only returns the childs of a DOMElement and doesn't 
        // support xPath. For this reason we need the XercesDomParser in this object.
        XercesXMLChPtr xPath(XMLString::transcode(name));
        XercesPtr<DOMXPathResult> result(m_parser->getDocument()->evaluate(
            xPath.Get(),
            m_element,
            m_resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr));

        std::vector<ComPtr<IMsixElement>> elementsEnum;
        for (XMLSize_t i = 0; i < result->getSnapshotLength(); i++)
        {
            result->snapshotItem(i);
            auto node = static_cast<DOMElement*>(result->getNodeValue());
            auto item = ComPtr<IMsixElement>::Make<XercesElement>(m_factory, node, m_parser);
            elementsEnum.push_back(std::move(item));
        }
        *elements = ComPtr<IMsixElementEnumerator>::
            Make<EnumeratorCom<IMsixElementEnumerator,IMsixElement>>(elementsEnum).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

protected:
    IMsixFactory* m_factory = nullptr;
    DOMElement* m_element = nullptr;
    XERCES_CPP_NAMESPACE::XercesDOMParser* m_parser;
    XercesPtr<DOMXPathNSResolver> m_resolver;
};

class XercesDom final : public ComClass<XercesDom, IXmlDom>
{
public:
    XercesDom(IMsixFactory* factory, const ComPtr<IStream>& stream, XmlContentType footPrintType) :
        m_factory(factory), m_stream(stream)
    {
        auto buffer = Helper::CreateBufferFromStream(stream);
        std::unique_ptr<XERCES_CPP_NAMESPACE::MemBufInputSource> source = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(&buffer[0]), buffer.size(), "XML File");

        // Create parser and grammar pool
        auto grammarPool = std::make_unique<XERCES_CPP_NAMESPACE::XMLGrammarPoolImpl>(XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager);
        m_parser = std::make_unique<XERCES_CPP_NAMESPACE::XercesDOMParser>(nullptr, XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager, grammarPool.get());
        
        // For Non validation parser GetResources will return an empty vector for the ContentType, BlockMap and AppxBundleManifest.
        // XercesDom will only parse the schemas if the vector is not empty. If not, it will only see that it is valid xml.
        std::vector<std::pair<std::string, ComPtr<IStream>>> schemas;
        if (footPrintType == XmlContentType::AppxBlockMapXml)
        {
            schemas = GetResources(m_factory, Resource::Type::BlockMap);
        }
        else if (footPrintType == XmlContentType::AppxManifestXml)
        {
            schemas = GetResources(m_factory, Resource::Type::AppxManifest);
        }
        else if (footPrintType == XmlContentType::ContentTypeXml)
        {
            schemas = GetResources(m_factory, Resource::Type::ContentType);
        }
        else if (footPrintType == XmlContentType::AppxBundleManifestXml)
        {
            schemas = GetResources(m_factory, Resource::Type::AppxBundleManifest);
        }
        else
        {
            ThrowError(Error::InvalidParameter);
        }

        // Set the error handler for the parser
        auto errorHandler = std::make_unique<ParsingException>();
        m_parser->setErrorHandler(errorHandler.get());

        auto entityResolver = std::make_unique<MsixEntityResolver>(m_factory);

        if (footPrintType == XmlContentType::AppxManifestXml || footPrintType == XmlContentType::AppxBundleManifestXml)
        {
            m_parser->setXMLEntityResolver(entityResolver.get());
        }

        bool HasSchemas = !schemas.empty();
        m_parser->setValidationScheme(HasSchemas ? 
            XERCES_CPP_NAMESPACE::AbstractDOMParser::ValSchemes::Val_Always : 
            XERCES_CPP_NAMESPACE::AbstractDOMParser::ValSchemes::Val_Never
        );
        m_parser->cacheGrammarFromParse(HasSchemas);
        m_parser->setDoSchema(HasSchemas);
        m_parser->setDoNamespaces(HasSchemas);
        m_parser->setHandleMultipleImports(HasSchemas); // TODO: do we need to handle the case where there aren't multiple schemas with the same namespace?
        m_parser->setValidationSchemaFullChecking(HasSchemas);

        if (HasSchemas)
        {   // Disable DTD and prevent XXE attacks.  See https://www.owasp.org/index.php/XML_External_Entity_(XXE)_Prevention_Cheat_Sheet#libxerces-c for additional details.
            m_parser->setIgnoreCachedDTD(true);
            m_parser->setSkipDTDValidation(true);
            m_parser->setCreateEntityReferenceNodes(false);
        }

        // Add schemas
        if (HasSchemas)
        {
            for(auto& schema : schemas)
            {
                auto schemaBuffer = Helper::CreateBufferFromStream(schema.second);
                auto item = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
                    reinterpret_cast<const XMLByte*>(&schemaBuffer[0]), schemaBuffer.size(), schema.first.c_str());
                m_parser->loadGrammar(*item, XERCES_CPP_NAMESPACE::Grammar::GrammarType::SchemaGrammarType, true);
            }
        }

        m_parser->parse(*source);

        m_resolver = XercesPtr<DOMXPathNSResolver>(m_parser->getDocument()->createNSResolver(m_parser->getDocument()));
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        return ComPtr<IXmlElement>::Make<XercesElement>(m_factory, m_parser->getDocument()->getDocumentElement(), m_parser.get());
    }

    bool ForEachElementIn(const ComPtr<IXmlElement>& root, XmlQueryName query, XmlVisitor& visitor) override
    {
        ComPtr<IXercesElement> element = root.As<IXercesElement>();

        XercesXMLChPtr xPath(XMLString::transcode(xPaths[static_cast<uint8_t>(query)]));
        XercesPtr<DOMXPathResult> result(m_parser->getDocument()->evaluate(
            xPath.Get(),
            element->GetElement(),
            m_resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr));
        
        for (XMLSize_t i = 0; i < result->getSnapshotLength(); i++)
        {
            result->snapshotItem(i);
            auto node = static_cast<DOMElement*>(result->getNodeValue());
            auto item = ComPtr<IXmlElement>::Make<XercesElement>(m_factory, node, m_parser.get());
            if (!visitor.Callback(visitor.context, item))
            {
                return false;
            }
        }
        return true;
    }

protected:
    IMsixFactory* m_factory;
    std::unique_ptr<XERCES_CPP_NAMESPACE::XercesDOMParser> m_parser;
    XercesPtr<DOMXPathNSResolver> m_resolver;
    ComPtr<IStream> m_stream;
};

class XercesFactory final : public ComClass<XercesFactory, IXmlFactory>
{
public:
    XercesFactory(IMsixFactory* factory) : m_factory(factory)
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
    }

    ~XercesFactory()
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
    }

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const ComPtr<IStream>& stream) override
    {
        return ComPtr<IXmlDom>::Make<XercesDom>(m_factory, stream, footPrintType);
    }
protected:
    IMsixFactory* m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMsixFactory* factory) { return ComPtr<IXmlFactory>::Make<XercesFactory>(factory); }

} // namespace MSIX