#pragma once
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"
#include "UnicodeConversion.hpp"

#include <msxml6.h>

EXTERN_C const IID IID_IMSXMLElement;

#ifndef WIN32
// {2730f595-0c80-4f3e-8891-753b2e8c305d}
interface IMSXMLElement : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IMSXMLElement : public IUnknown
#endif
// An internal interface for XML document object model
{
public:
    virtual IXMLDOMElement* GetElement() = 0;
};

SpecializeUuidOfImpl(IMSXMLElement);

namespace MSIX {

//              content type ->              alias  <-> uri
static std::map<XmlContentType, std::vector<std::pair<std::wstring, std::wstring>>> xmlNamespaces = {
     {XmlContentType::ContentTypeXml,    {}},
     {XmlContentType::AppxBlockMapXml,   {
         {L"a", L"http://schemas.microsoft.com/appx/2010/blockmap"},
         {L"b", L"http://schemas.microsoft.com/appx/2015/blockmap"},
         {L"c", L"http://schemas.microsoft.com/appx/2017/blockmap"},
     }},
     {XmlContentType::AppxManifestXml,   {
     }}
 };

static std::map<XmlQueryName, std::wstring> xPaths = {
    {XmlQueryName::Package_Identity                             ,L"Package/Identity"},
    {XmlQueryName::BlockMap_File                                ,L"/*[local-name()='BlockMap']/*[local-name()='File']"},
    {XmlQueryName::BlockMap_File_Block                          ,L"*[local-name()='Block']"},
};    

static std::map<XmlAttributeName, std::wstring> attributeNames = {
    {XmlAttributeName::Package_Identity_Name                    ,L"Name"},
    {XmlAttributeName::Package_Identity_ProcessorArchitecture   ,L"ProcessorArchitecture"},
    {XmlAttributeName::Package_Identity_Publisher               ,L"Publisher"},
    {XmlAttributeName::Package_Identity_Version                 ,L"Version"},
    {XmlAttributeName::Package_Identity_ResourceId              ,L"ResourceId"},

    {XmlAttributeName::BlockMap_File_Name                       ,L"Name"},
    {XmlAttributeName::BlockMap_File_LocalFileHeaderSize        ,L"LfhSize"},
    {XmlAttributeName::BlockMap_File_Block_Size                 ,L"Size"},
    {XmlAttributeName::BlockMap_File_Block_Hash                 ,L"Hash"},
};


const std::uint8_t base64DecoderRing[128] =
{
    /*    0-15 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*   16-31 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*   32-47 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
    /*   48-63 */   52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xFF, 0xFF, 0xFF,   64, 0xFF, 0xFF,
    /*   64-79 */ 0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    /*   80-95 */   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*  96-111 */ 0xFF,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    /* 112-127 */   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

class Bstr
{
    BSTR m_bstr;
public:
    operator BSTR() && = delete;
    operator BSTR() & { return m_bstr; }
    Bstr() {m_bstr = nullptr;}
    Bstr(std::wstring text)
    {
        m_bstr = ::SysAllocStringLen(text.c_str(), text.length()); 
        ThrowErrorIf(Error::OutOfMemory, (m_bstr == nullptr), "SysAllocStringLen failed!");
    }
    ~Bstr() { ::SysFreeString(m_bstr);}

    BSTR* AddressOf()
    { 
        ::SysFreeString(m_bstr);
        return &m_bstr;
    }    

    BSTR& Get() { return m_bstr; }
};

class Variant
{
    VARIANT m_variant; 
public:
    operator VARIANT() && = delete;
    operator VARIANT() & { return m_variant; }
    Variant() { VariantInit(&m_variant); }
    ~Variant() { VariantClear(&m_variant); }

    VARIANT* AddressOf()
    { 
        VariantClear(&m_variant);
        return &m_variant;
    }

    VARIANT& Get() { return m_variant; }
};

class MSXMLElement : public ComClass<MSXMLElement, IXmlElement, IMSXMLElement>
{
    Variant GetAttribute(XmlAttributeName attribute)
    {
        Bstr name(attributeNames[attribute]);
        Variant value;
        ThrowHrIfFailed(m_element->getAttribute(name, value.AddressOf()));
        ThrowErrorIfNot(Error::Unexpected, (value.Get().vt == VT_BSTR), "unexpected attribute value type.");    
        return std::move(value);    
    }

public:
    MSXMLElement(ComPtr<IXMLDOMElement>& element) : m_element(element) {}

    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        return utf16_to_utf8(static_cast<WCHAR*>(GetAttribute(attribute).Get().bstrVal));
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        std::vector<std::uint8_t> result;
        auto intermediate = GetAttributeValue(attribute);
        ThrowErrorIfNot(Error::InvalidParameter, (0 == (intermediate.length() % 4)), "invalid base64 encoding");
        for(std::size_t index=0; index < intermediate.length(); index += 4)
        {
            ThrowErrorIf(Error::InvalidParameter,(
            (intermediate[index+0] | intermediate[index+1] | intermediate[index+2] | intermediate[index+3]) >= 128
            ), "invalid base64 encoding");

            ULONG v1 = base64DecoderRing[intermediate[index+0]];
            ULONG v2 = base64DecoderRing[intermediate[index+1]];
            ULONG v3 = base64DecoderRing[intermediate[index+2]];
            ULONG v4 = base64DecoderRing[intermediate[index+3]];

            ThrowErrorIf(Error::InvalidParameter,(((v1 | v2) >= 64) || ((v3 | v4) == 0xFF)), "first two chars of a four char base64 sequence can't be ==, and must be valid");
            ThrowErrorIf(Error::InvalidParameter,(v3 == 64 && v4 != 64), "if the third char is = then the fourth char must be =");
            std::size_t byteCount = (v4 != 64 ? 3 : (v3 != 64 ? 2 : 1));
            result.push_back(static_cast<std::uint8_t>(((v1 << 2) | ((v2 >> 4) & 0x03))));
            if (byteCount >1)
            {
                result.push_back(static_cast<std::uint8_t>(((v2 << 4) | ((v3 >> 2) & 0x0F)) & 0xFF));
                if (byteCount >2)
                {
                    result.push_back(static_cast<std::uint8_t>(((v3 << 6) | ((v4 >> 0) & 0x3F)) & 0xFF));
                }
            }
        }
        return result;
    }

    // IMSXMLElement
    IXMLDOMElement* GetElement() override { return m_element.Get(); }

protected:
    ComPtr<IXMLDOMElement> m_element;
};

class MSXMLDom : public ComClass<MSXMLDom, IXmlDom>
{
public:
    MSXMLDom(ComPtr<IStream>& stream, std::vector<std::pair<std::wstring, std::wstring>>& namespaces)
    {
        ThrowHrIfFailed(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&xmlDocument)));
        ThrowHrIfFailed(xmlDocument->put_async(VARIANT_FALSE));

        VARIANT vTrue;
        VariantInit(&vTrue);
        vTrue.vt = VT_BOOL;
        vTrue.boolVal = VARIANT_TRUE;
        Bstr property(L"NewParser"); // huh?
        ThrowHrIfFailed(xmlDocument->setProperty(property.Get(), vTrue));

        VARIANT var;
        var.vt = VT_UNKNOWN;
        var.punkVal = stream.Get();

        VARIANT_BOOL success(VARIANT_FALSE);
        ThrowHrIfFailed(xmlDocument->load(var, &success));
        if (VARIANT_FALSE == success)
        {
            ComPtr<IXMLDOMParseError> error;            
            ThrowHrIfFailed(xmlDocument->get_parseError(&error));

            long errorCode = 0, lineNumber = 0, columnNumber = 0;
            ThrowHrIfFailed(error->get_errorCode(&errorCode));
            ThrowHrIfFailed(error->get_line(&lineNumber));
            ThrowHrIfFailed(error->get_linepos(&columnNumber));

            Bstr reason;
            ThrowHrIfFailed(error->get_reason(reason.AddressOf()));
            std::ostringstream message;
            message << "Error: " << errorCode << " on [" << lineNumber << ":" << columnNumber << "]: " << utf16_to_utf8(static_cast<wchar_t*>(reason.Get()));
            ThrowErrorIf(Error::Unexpected, (success == VARIANT_FALSE), message.str());
        }
        
        size_t countNamespaces = 0;
        std::wostringstream value;
        for(auto& item : namespaces)
        {               
            if (0 != countNamespaces) { value << L" "; } // namespaces are space-delimited
            value << L"xmlns:" << item.first << LR"(=")" << item.second << LR"(")";
            countNamespaces++;
        }

        if (countNamespaces != 0)
        {
            Bstr selectionProperty(L"SelectionNamespaces");
            Bstr selectionValue(value.str());
            VARIANT var;
            var.vt = VT_BSTR;
            var.bstrVal = selectionValue.Get();
            ThrowHrIfFailed(xmlDocument->setProperty(selectionProperty, var));
        }
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        ComPtr<IXMLDOMElement> element;
        ThrowHrIfFailed(xmlDocument->get_documentElement(&element));
        return ComPtr<IXmlElement>::Make<MSXMLElement>(element);
    }
    
    bool ForEachElementIn(IXmlElement* root, XmlQueryName query, std::function<bool(IXmlElement*)> visitor) override
    {
        ComPtr<IMSXMLElement> element;
        ThrowHrIfFailed(root->QueryInterface(UuidOfImpl<IMSXMLElement>::iid, reinterpret_cast<void**>(&element)));

        Bstr xPath(xPaths[query]);
        ComPtr<IXMLDOMNodeList> list;
        ThrowHrIfFailed(element->GetElement()->selectNodes(xPath, &list));

        long count = 0;
        ThrowHrIfFailed(list->get_length(&count));
        for(long index=0; index < count; index++)
        {
            ComPtr<IXMLDOMNode> node;
            ThrowHrIfFailed(list->get_item(index, &node));
            ComPtr<IXMLDOMElement> elementItem;
            ThrowHrIfFailed(node->QueryInterface(__uuidof(IXMLDOMElement), reinterpret_cast<void**>(&elementItem)));
            auto item = ComPtr<IXmlElement>::Make<MSXMLElement>(elementItem);
            if (!visitor(item.Get()))
            {
                return false;
            }
        }
        return true;
    }

protected:
    ComPtr<IXMLDOMDocument2> xmlDocument;
};

class MSXMLFactory : public ComClass<MSXMLFactory, IXmlFactory>
{
public:
    MSXMLFactory()
    {
        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        m_CoInitialized = SUCCEEDED(result);
        if (RPC_E_CHANGED_MODE == result) { result = S_FALSE; }
        ThrowHrIfFailed(result);
    }
    ~MSXMLFactory() { if (m_CoInitialized) { CoUninitialize(); } }

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, ComPtr<IStream>& stream) override
    {        
        switch (footPrintType)
        {   // TODO: pass schemas for validation.
            case XmlContentType::AppxBlockMapXml:
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType]);
            case XmlContentType::AppxManifestXml:
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType]);
            case XmlContentType::ContentTypeXml:
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType]);
        }
        throw Exception(Error::InvalidParameter);    
    }

protected:
    bool m_CoInitialized = false;
};

ComPtr<IXmlFactory> CreateXmlFactory() { return ComPtr<IXmlFactory>::Make<MSXMLFactory>(); }

} // namespace MSIX