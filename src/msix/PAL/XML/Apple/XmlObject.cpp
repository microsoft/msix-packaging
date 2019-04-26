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
#include "Encoding.hpp"
#include "StreamHelper.hpp"
#include "MSIXResource.hpp"
#include "UnicodeConversion.hpp"
#include "Enumerators.hpp"
#include "XmlDocumentReader.hpp"

// An internal interface for apple XML document object model
// {8FBC0096-E87D-406A-95D9-203ADEFBF9AF}
interface IAppleXmlElement : public IUnknown
{
public:
    virtual MSIX::XmlNode* GetXmlNode() = 0;
};
MSIX_INTERFACE(IAppleXmlElement,  0x8fbc0096, 0xe87d, 0x406a, 0x95, 0xd9, 0x20, 0x3a, 0xde, 0xfb, 0xf9, 0xaf);

namespace MSIX {

class XmlElement final : public ComClass<XmlElement, IXmlElement, IAppleXmlElement, IMsixElement>
{
public:
    XmlElement(IMsixFactory* factory, XmlNode* xmlNode) :
        m_factory(factory), m_xmlNode(xmlNode)
    {
    }

    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        auto intermediate = GetAttributeNameStringUtf8(attribute);
        return GetAttributeValue(intermediate);
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        auto intermediate = GetAttributeValue(attribute);
        return Encoding::GetBase64DecodedValue(intermediate);
    }

    std::string GetText() override
    {
        return m_xmlNode->Text;
    }

    // IAppleXmlElement
    XmlNode* GetXmlNode() override { return m_xmlNode; }

     // IMsixElement
    HRESULT STDMETHODCALLTYPE GetAttributeValue(LPCWSTR name, LPWSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        auto intermediate = wstring_to_utf8(name);
        auto attributeValue = GetAttributeValue(intermediate);
        return m_factory->MarshalOutString(attributeValue, value);;
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
        return m_factory->MarshalOutStringUtf8(attributeValue, value);;
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
        auto attribute = std::string(name);
        auto elementsFound = m_xmlNode->FindElements(attribute);
        std::vector<ComPtr<IMsixElement>> elementsEnum;
        for(auto element : elementsFound)
        {
            auto item = ComPtr<IMsixElement>::Make<XmlElement>(m_factory, element);
            elementsEnum.push_back(std::move(item));
        }
        *elements = ComPtr<IMsixElementEnumerator>::Make<EnumeratorCom<IMsixElementEnumerator,IMsixElement>>(elementsEnum).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

private:
    IMsixFactory* m_factory = nullptr;
    XmlNode* m_xmlNode = nullptr;

    std::string GetAttributeValue(std::string& attributeName)
    {
        return m_xmlNode->Attributes[attributeName];
    }
};

class XmlDom final : public ComClass<XmlDom, IXmlDom>
{
public:
    XmlDom(IMsixFactory* factory, const ComPtr<IStream>& stream) :
        m_factory(factory), m_stream(stream)
    {
        auto buffer = Helper::CreateBufferFromStream(stream);

        m_xmlDocumentReader.reset(new XmlDocumentReader());
        m_xmlDocumentReader->Init();
        ThrowErrorIfNot(MSIX::Error::XmlFatal, m_xmlDocumentReader->Parse(buffer.data(), buffer.size()), "Xml Parse failed.");

        // Apple currently only supports SAX parser.
        // If schema validation is required, then use xerces as the xml parser.
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        return ComPtr<IXmlElement>::Make<XmlElement>(m_factory, m_xmlDocumentReader->GetRoot());
    }

    bool ForEachElementIn(const ComPtr<IXmlElement>& root, XmlQueryName query, XmlVisitor& visitor) override
    {
        ComPtr<IAppleXmlElement> element = root.As<IAppleXmlElement>();
        XmlNode* xmlNode = element->GetXmlNode();
        auto elements = xmlNode->FindElements(GetQueryString(query));

         for(auto element : elements)
         {
            auto item = ComPtr<IXmlElement>::Make<XmlElement>(m_factory, element);
            if (!visitor.Callback(visitor.context, item))
            {
                return false;
            }
        }
        return true;
    }

protected:
    IMsixFactory* m_factory;
    ComPtr<IStream> m_stream;
    std::unique_ptr<XmlDocumentReader> m_xmlDocumentReader;
};

class AppleXmlFactory final : public ComClass<AppleXmlFactory, IXmlFactory>
{
public:
    AppleXmlFactory(IMsixFactory* factory) : m_factory(factory)
    {
    }

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const ComPtr<IStream>& stream) override
    {
        return ComPtr<IXmlDom>::Make<XmlDom>(m_factory, stream);
    }
protected:
    IMsixFactory* m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMsixFactory* factory) { return ComPtr<IXmlFactory>::Make<AppleXmlFactory>(factory); }

} // namespace MSIX