#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"
#include "StreamHelper.hpp"
#include "MSIXResource.hpp"

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

XERCES_CPP_NAMESPACE_USE

EXTERN_C const IID IID_IXercesElement;

#ifndef WIN32
// {07d6ee0e-2165-4b90-8024-e176291e77dd}
interface IXercesElement : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IXercesElement : public IUnknown
#endif
// An internal interface for XML document object model
{
public:
    virtual DOMElement* GetElement() = 0;
};

SpecializeUuidOfImpl(IXercesElement);

namespace MSIX {

static std::map<XmlQueryName, std::string> xPaths = {
    {XmlQueryName::Package_Identity                             ,"/Package/Identity"},
    {XmlQueryName::BlockMap_File                                ,"/BlockMap/File"},
    {XmlQueryName::BlockMap_File_Block                          ,"./Block"},
};

static std::map<XmlAttributeName, std::string> attributeNames = {
    {XmlAttributeName::Package_Identity_Name                    ,"Name"},
    {XmlAttributeName::Package_Identity_ProcessorArchitecture   ,"ProcessorArchitecture"},
    {XmlAttributeName::Package_Identity_Publisher               ,"Publisher"},
    {XmlAttributeName::Package_Identity_Version                 ,"Version"},
    {XmlAttributeName::Package_Identity_ResourceId              ,"ResourceId"},

    {XmlAttributeName::BlockMap_File_Name                       ,"Name"},
    {XmlAttributeName::BlockMap_File_LocalFileHeaderSize        ,"LfhSize"},
    {XmlAttributeName::BlockMap_File_Block_Size                 ,"Size"},
    {XmlAttributeName::BlockMap_File_Block_Hash                 ,"Hash"},
};

class ParsingException : public XERCES_CPP_NAMESPACE::ErrorHandler
{
public:
    ParsingException() {};
    ~ParsingException() {};

    void warning(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {
        // TODO: add message, line number and column
        assert(false);
        throw Exception(MSIX::Error::XercesWarning);
    }

    void error(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {
        // TODO: add message, line number and column
        assert(false);
        throw Exception(MSIX::Error::XercesError);
    }

    void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exp) override
    {
        // TODO: add message, line number and column
        assert(false);
        throw Exception(MSIX::Error::XercesFatal);
    }

    void resetErrors() override {}
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

class XercesElement : public ComClass<XercesElement, IXmlElement, IXercesElement>
{
public:
    XercesElement(DOMElement* element) : m_element(element) {}
    
    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(attributeNames[attribute].c_str()));
        XercesCharPtr value(XMLString::transcode(m_element->getAttribute(nameAttr.Get())));
        return std::string(value.Get());
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(attributeNames[attribute].c_str()));
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

    // IXercesElement
    DOMElement* GetElement() override { return m_element; }

protected:
    DOMElement* m_element = nullptr;
};

class XercesDom : public ComClass<XercesDom, IXmlDom>
{
public:
    XercesDom(ComPtr<IStream>& stream, std::vector<ComPtr<IStream>>* schemas = nullptr) :  m_stream(stream)
    {
        auto buffer = Helper::CreateBufferFromStream(stream);
        std::unique_ptr<XERCES_CPP_NAMESPACE::MemBufInputSource> source = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(&buffer[0]), buffer.size(), "XML File");

        // Create parser and grammar pool
        auto grammarPool = std::make_unique<XERCES_CPP_NAMESPACE::XMLGrammarPoolImpl>(XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager);
        m_parser = std::make_unique<XERCES_CPP_NAMESPACE::XercesDOMParser>(nullptr, XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager, grammarPool.get());
        
        bool HasSchemas = ((schemas != nullptr) && (schemas->begin() != schemas->end()));
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
        {   for(auto& schema : *schemas)
            {   auto schemaBuffer = Helper::CreateBufferFromStream(schema);
                auto item = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
                    reinterpret_cast<const XMLByte*>(&schemaBuffer[0]), schemaBuffer.size(), "Schema");
                m_parser->loadGrammar(*item, XERCES_CPP_NAMESPACE::Grammar::GrammarType::SchemaGrammarType, true);
            }           
        }

        // Set the error handler for the parser
        auto errorHandler = std::make_unique<ParsingException>();
        m_parser->setErrorHandler(errorHandler.get());
        m_parser->parse(*source);            
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        return ComPtr<IXmlElement>::Make<XercesElement>(m_parser->getDocument()->getDocumentElement());
    }

    bool ForEachElementIn(IXmlElement* root, XmlQueryName query, std::function<bool(IXmlElement*)> visitor) override
    {
        ComPtr<IXercesElement> element;
        ThrowHrIfFailed(root->QueryInterface(UuidOfImpl<IXercesElement>::iid, reinterpret_cast<void**>(&element)));

        XercesXMLChPtr xPath(XMLString::transcode(xPaths[query].c_str()));
        // TODO: Do we need to always create a resolver, or can we reuse it once we create one?
        XercesPtr<DOMXPathNSResolver> resolver(m_parser->getDocument()->createNSResolver(m_parser->getDocument()));
        XercesPtr<DOMXPathResult> result(m_parser->getDocument()->evaluate(
            xPath.Get(),
            element->GetElement(),
            resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr));
        
        for (XMLSize_t i = 0; i < result->getSnapshotLength(); i++)
        {
            result->snapshotItem(i);
            auto node = static_cast<DOMElement*>(result->getNodeValue());
            auto item = ComPtr<IXmlElement>::Make<XercesElement>(node);
            if (!visitor(item.Get()))
            {
                return false;
            }
        }
        return true;
    }

protected:
    std::unique_ptr<XERCES_CPP_NAMESPACE::XercesDOMParser> m_parser;
    ComPtr<IStream> m_stream;    
};

class XercesFactory : public ComClass<XercesFactory, IXmlFactory>
{
public:
    XercesFactory(IMSIXFactory* factory) : m_factory(factory)
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
    }

    ~XercesFactory()
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
    }

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, ComPtr<IStream>& stream) override
    {
        switch (footPrintType)
        {
            case XmlContentType::AppxBlockMapXml:
            {
                auto blockMapSchema = GetResources(m_factory, Resource::Type::BlockMap);
                return ComPtr<IXmlDom>::Make<XercesDom>(stream, &blockMapSchema);
            }
            case XmlContentType::AppxManifestXml:
                // TODO: pass schemas to validate AppxManifest. This only validates that is a well-formed xml
                return ComPtr<IXmlDom>::Make<XercesDom>(stream);
            case XmlContentType::ContentTypeXml:
            {
                auto contentTypeSchema = GetResources(m_factory, Resource::Type::ContentType);
                return ComPtr<IXmlDom>::Make<XercesDom>(stream, &contentTypeSchema);
            }
        }
        throw Exception(Error::InvalidParameter);
    }
protected:
    IMSIXFactory* m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMSIXFactory* factory) { return ComPtr<IXmlFactory>::Make<XercesFactory>(factory); }

} // namespace MSIX