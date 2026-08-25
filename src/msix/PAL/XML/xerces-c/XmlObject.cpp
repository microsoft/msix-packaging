//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>

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
#include "xercesc/util/XMLUni.hpp" // helpful XMLChr*
#include "xercesc/framework/MemBufFormatTarget.hpp"

XERCES_CPP_NAMESPACE_USE

// An internal interface for XML document object model
// {07d6ee0e-2165-4b90-8024-e176291e77dd}
#ifndef WIN32
interface IXercesElement : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IXercesElement : public IUnknown
#endif
{
public:
    virtual DOMElement* GetElement() = 0;
    virtual std::string GetAttributeValue(const std::string& attributeName) = 0;
};
MSIX_INTERFACE(IXercesElement,  0x07d6ee0e,0x2165,0x4b90,0x80,0x24,0xe1,0x76,0x29,0x1e,0x77,0xdd);

namespace MSIX {

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
        std::u16string utf16FileName = std::u16string(const_cast<char16_t*>(reinterpret_cast<const char16_t*>(exp.getSystemId())));
        std::u16string utf16Message = std::u16string(const_cast<char16_t*>(reinterpret_cast<const char16_t*>(exp.getMessage())));
        return "Error in " + u16string_to_utf8(utf16FileName) + " [Line " + std::to_string(static_cast<std::uint64_t>(exp.getLineNumber()))
            + ", Col " + std::to_string(static_cast<std::uint64_t>(exp.getColumnNumber())) + "] :: " + u16string_to_utf8(utf16Message);
    }
};

class MsixEntityResolver : public XMLEntityResolver
{
public:
    MsixEntityResolver(IMsixFactory* factory, const NamespaceManager& namespaces) : m_factory(factory), m_namespaces(namespaces) {}
    ~MsixEntityResolver() {}

    InputSource* resolveEntity(XMLResourceIdentifier* resourceIdentifier)
    {
        std::u16string utf16string = std::u16string(const_cast<char16_t*>(reinterpret_cast<const char16_t*>(resourceIdentifier->getNameSpace())));
        std::string id = u16string_to_utf8(utf16string);
        const auto& entry = std::find(m_namespaces.begin(), m_namespaces.end(), id.c_str());
        ThrowErrorIf(MSIX::Error::XmlError, entry == m_namespaces.end(), "Invalid namespace");
        auto stream = m_factory->GetResource(entry->schema);
        auto schemaBuffer = Helper::CreateRawBufferFromStream(stream);
        auto item = std::make_unique<MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(schemaBuffer.second.release()), schemaBuffer.first, entry->schema, true /*delete by xerces*/);
        return item.release();
    }
private:
    const NamespaceManager m_namespaces;
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
public:

    XercesElement(IMsixFactory* factory, DOMElement* element, XERCES_CPP_NAMESPACE::XercesDOMParser* parser) :
        m_factory(factory), m_element(element), m_parser(parser)
    {
        m_resolver = XercesPtr<DOMXPathNSResolver>(m_parser->getDocument()->createNSResolver(m_parser->getDocument()));
    }
    
    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        auto attributeName = GetAttributeNameStringUtf8(attribute);
        return GetAttributeValue(attributeName);
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(GetAttributeNameStringUtf8(attribute)));
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
        if (value.Get() != nullptr)
        {
            return std::string(value.Get());
        }
        return {};
    }

    std::string GetPrefix() override
    {
        DOMNode* node = dynamic_cast<DOMNode*>(m_element);
        XercesCharPtr value(XMLString::transcode(node->getPrefix()));
        if (value.Get() != nullptr)
        {
            return std::string(value.Get());
        }
        return {};
    }

    // IXercesElement
    DOMElement* GetElement() override { return m_element; }

    std::string GetAttributeValue(const std::string& attributeName) override
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(attributeName.c_str()));
        auto utf16string = std::u16string(const_cast<char16_t*>(reinterpret_cast<const char16_t*>(m_element->getAttribute(nameAttr.Get()))));
        return u16string_to_utf8(utf16string);
    }

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

    HRESULT STDMETHODCALLTYPE GetElements(LPCWSTR xpath, IMsixElementEnumerator** elements) noexcept override try
    {
        return GetElementsUtf8(wstring_to_utf8(xpath).c_str(), elements);
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

    HRESULT STDMETHODCALLTYPE GetElementsUtf8(LPCSTR xpath, IMsixElementEnumerator** elements) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (elements == nullptr || *elements != nullptr), "bad pointer.");
        // Note: getElementsByTagName only returns the childs of a DOMElement and doesn't 
        // support xPath. For this reason we need the XercesDomParser in this object.
        XercesXMLChPtr xPath(XMLString::transcode(xpath));
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
            // Block map xml does not need schema validation.
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

        // Set the error handler and entity resolver for the parser
        auto errorHandler = std::make_unique<ParsingException>();
        auto entityResolver = std::make_unique<MsixEntityResolver>(m_factory, s_xmlNamespaces[static_cast<std::uint8_t>(footPrintType)]);
        m_parser->setErrorHandler(errorHandler.get());
        m_parser->setXMLEntityResolver(entityResolver.get());
        m_parser->setDoNamespaces(true);

        if (!schemas.empty())
        {
            if (footPrintType == XmlContentType::AppxManifestXml || footPrintType == XmlContentType::AppxBundleManifestXml)
            {
                source = StripIgnorableNamespaces(*source, s_xmlNamespaces[static_cast<std::uint8_t>(footPrintType)]);
            }

            m_parser->setValidationScheme(XERCES_CPP_NAMESPACE::AbstractDOMParser::ValSchemes::Val_Always);
            m_parser->cacheGrammarFromParse(true);
            m_parser->setDoSchema(true);
            m_parser->setValidationSchemaFullChecking(true);
            // Disable DTD and prevent XXE attacks.  See https://www.owasp.org/index.php/XML_External_Entity_(XXE)_Prevention_Cheat_Sheet#libxerces-c for additional details.
            m_parser->setIgnoreCachedDTD(true);
            m_parser->setSkipDTDValidation(true);
            m_parser->setCreateEntityReferenceNodes(true);
            m_parser->setDisableDefaultEntityResolution(true);

            for(const auto& schema : schemas)
            {
                auto schemaBuffer = Helper::CreateBufferFromStream(schema.second);
                auto item = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
                    reinterpret_cast<const XMLByte*>(&schemaBuffer[0]), schemaBuffer.size(), schema.first.c_str());
                m_parser->loadGrammar(*item, XERCES_CPP_NAMESPACE::Grammar::GrammarType::SchemaGrammarType, true);
            }
        }

        m_parser->parse(*source);

        // TODO: Do semantic check for all the elements we modified to maxOcurrs=unbounded and xs:patterns
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        return ComPtr<IXmlElement>::Make<XercesElement>(m_factory, m_parser->getDocument()->getDocumentElement(), m_parser.get());
    }

    bool ForEachElementIn(const ComPtr<IXmlElement>& root, XmlQueryName query, XmlVisitor& visitor) override
    {
        DOMElement* element = root.As<IXercesElement>()->GetElement();

        std::list<DOMElement*> list;
        std::string xpath(GetQueryString(query));

        if (xpath.size() >= 2 && xpath[0] == '.' && xpath[1] == '/')
        {
            FindChildElements(xpath.substr(2), element, list);
        }
        else if (xpath.size() > 1 && xpath[0] == '/')
        {
            // Get name of root element
            std::size_t secondSlash =  xpath.find_first_of('/', 1);
            XercesXMLChPtr firstElement(XMLString::transcode(xpath.substr(1, secondSlash - 1).c_str()));
            if (XMLString::compareString(firstElement.Get(), static_cast<DOMNode*>(element)->getLocalName()) == 0)
            {
                FindChildElements(xpath.substr(secondSlash + 1), element, list);
            }
            else
            {
                ThrowErrorAndLog(Error::XmlFatal, "Invalid root element");
            }
        }

        for(const auto& element : list)
        {
            auto item = ComPtr<IXmlElement>::Make<XercesElement>(m_factory, element, m_parser.get());
            if (!visitor(item))
            {
                return false;
            }
        }
        return true;
    }

protected:

    std::unique_ptr<MemBufInputSource> StripIgnorableNamespaces(const InputSource& source, const NamespaceManager& namespaces)
    {
        m_parser->setDoNamespaces(true);
        m_parser->parse(source);
        XERCES_CPP_NAMESPACE::DOMDocument* dom = m_parser->getDocument();
        auto rootElement = ComPtr<IXercesElement>::Make<XercesElement>(m_factory, dom->getDocumentElement(), m_parser.get());
        std::string attr = "IgnorableNamespaces";
        std::string attrValue = rootElement->GetAttributeValue(attr);
        if (!attrValue.empty())
        {
            std::vector<std::string> aliasesToLookup;
            {
                std::string alias;
                std::istringstream aliases(attrValue);
                while(getline(aliases, alias, ' ')) { aliasesToLookup.push_back(alias); }
            }
            for (const auto& a : aliasesToLookup)
            {
                std::string alias = "xmlns:" + a; // Look for xmlns:[alias] attribute name
                std::string aliasValue = rootElement->GetAttributeValue(alias);
                const auto& entry = std::find(namespaces.begin(), namespaces.end(), aliasValue.c_str());
                if (entry == namespaces.end()) // only strip if we don't know about it
                {
                    RemoveAllInNamespace(rootElement, a);
                }
            }

        }

        // Serialize the new dom to parse.
        static const XMLCh cs[3] = {chLatin_L, chLatin_S, chNull};
        DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(cs);
        auto serializer = XercesPtr<DOMLSSerializer>((static_cast<DOMImplementationLS*>(impl))->createLSSerializer());
        auto lsOutput = XercesPtr<DOMLSOutput>((static_cast<DOMImplementationLS*>(impl))->createLSOutput());

        // Set encoding to UTF-8
        static const XMLCh utf8Str[] = {chLatin_U, chLatin_T, chLatin_F, chDash, chDigit_8, chNull};
        lsOutput->setEncoding(utf8Str);

        std::unique_ptr<MemBufFormatTarget> formatTarget = std::make_unique<MemBufFormatTarget>();
        lsOutput->setByteStream(static_cast<XMLFormatTarget*>(formatTarget.get()));
        serializer->write(dom, lsOutput.Get());

        m_parser->reset();

        // Copy buffer, don't use unique_ptr here, this buffer is going to be deleted by Xerces
        XMLSize_t size = formatTarget->getLen();
        XMLByte* newBuffer = new XMLByte[size];
        std::memcpy(reinterpret_cast<void*>(newBuffer), 
            reinterpret_cast<void*>(const_cast<XMLByte*>(formatTarget->getRawBuffer())),
            static_cast<size_t>(size));
        return std::make_unique<MemBufInputSource>(newBuffer, size, "XML File", true /*delete by xerces*/);
    }

    // Remove elements and attributes from a specified namespace. We don't use the Xerces xPath APIs for several
    // reasons:
    // 1 - XPathScannerForSchema::addToken on xercesxpath.cpp explicitly disallows node() as a valid token to matches 
    // elements and attribute nodes with one single xpath...
    // 2 - Xerces will throw XMLExcepts::XPath_NoAttrSelector ("selector cannot select attribute"). for //@<namespace>:*.
    // See XercesXPath::checkForSelectedAttributes in xercesxpath.cpp. Removing the checkForSelectedAttributes from
    // XercesXPath::XercesXPath will allow us to use the xpath but the result will be the element node, not the 
    // attribute one. This implies modifying xerces and then iteratate the attributes of the elements.
    // 
    // Because we don't want to modify xerces, we will iterate through all of the elements and look at their attributes.
    // If we are doing that, there's no point selecting all the elements in the namespace using xpath,
    // just remove them in the same pass.
    void RemoveAllInNamespace(ComPtr<IXercesElement>& rootElement, const std::string& prefix)
    {
        XercesXMLChPtr XercesPrefix(XMLString::transcode(prefix.c_str()));
        std::queue<DOMNode*> nodeQueue;
        nodeQueue.push(static_cast<DOMNode*>(rootElement->GetElement()));
        while (!nodeQueue.empty())
        {
            auto node = nodeQueue.front();

            // Remove node if is from the ignorable namespace, no need to look at its childs anymore
            if (node->getPrefix() != nullptr &&
                (XMLString::compareString(node->getPrefix(), XercesPrefix.Get()) == 0))
            {
                DOMNode* parentNode = node->getParentNode();
                ThrowErrorIfNot(Error::XmlError, parentNode, "We are trying to delete the root element!");
                parentNode->removeChild(node);
            }
            else
            {
                // Add childs to queue
                DOMNode* child = node->getFirstChild();
                while (child)
                {
                    if (child->getNodeType() == DOMNode::ELEMENT_NODE)
                    {
                        nodeQueue.push(child);
                    }
                    child=child->getNextSibling();
                }
                // See if this node has attributes in the ignorable namespace
                if (node->hasAttributes())
                {
                    // DOMElement::removeAttributeNS requires knowing the name of the attribute
                    // so we have to get all of them and look one by one
                    DOMNamedNodeMap* attributes = node->getAttributes();
                    for (XMLSize_t i = 0; i < attributes->getLength(); i++)
                    {
                        DOMNode* attribute = attributes->item(i);
                        if (attribute->getPrefix() != nullptr && 
                        (XMLString::compareString(attribute->getPrefix(), XercesPrefix.Get()) == 0))
                        {
                            static_cast<DOMElement*>(node)->removeAttributeNode(static_cast<DOMAttr*>(attribute));
                        }
                    }
                }
            }
            nodeQueue.pop();
        }
    }

    void FindChildElements(std::string xpath, DOMElement* root, std::list<DOMElement*>& list)
    {
        // The special value "*" matches all namespaces
        XercesXMLChPtr allNS(XMLString::transcode("*"));

        // Find next element to search
        std::size_t nextSeparator = xpath.find_first_of('/');
        XercesXMLChPtr nextElement(XMLString::transcode(xpath.substr(0, nextSeparator).c_str()));

        DOMNodeList* childs = root->getElementsByTagNameNS(allNS.Get(), nextElement.Get());
        XMLSize_t childsSize = childs->getLength();
        for(XMLSize_t i = 0; i < childsSize; i++)
        {
            DOMNode* node = childs->item(i);
            if (XMLString::compareString(nextElement.Get(), node->getLocalName()) == 0)
            {
                if (nextSeparator == std::string::npos)
                {
                    // This is the node we are looking for.
                    list.emplace_back(static_cast<DOMElement*>(node));
                }
                else
                {
                    FindChildElements(xpath.substr(nextSeparator + 1), static_cast<DOMElement*>(node), list);
                }
            }
        }
    }

    IMsixFactory* m_factory;
    std::unique_ptr<XERCES_CPP_NAMESPACE::XercesDOMParser> m_parser;
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