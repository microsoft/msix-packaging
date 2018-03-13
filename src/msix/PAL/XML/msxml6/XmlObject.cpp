#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>

#include "Exceptions.hpp"
#include "Log.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"
#include "UnicodeConversion.hpp"
#include "MSIXResource.hpp"

#include <msxml6.h>

EXTERN_C const IID IID_IMSXMLElement;
EXTERN_C const IID IID_IMSXMLDom;

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

#ifndef WIN32
// {b6bca5f0-c6c1-4409-85be-e476aabec19a}
interface IMSXMLDom : public IUnknown
#else
class IMSXMLDom : public IUnknown
#endif
// An internal interface for XML document object model
{
public:
    virtual MSIX::ComPtr<IXMLDOMDocument> GetDomDocument() = 0;
};

SpecializeUuidOfImpl(IMSXMLElement);
SpecializeUuidOfImpl(IMSXMLDom);

namespace MSIX {

//              content type -> list[alias <-> uri <-> XSD]
static std::map<XmlContentType, std::vector<std::tuple<std::wstring, std::wstring, std::string>>> xmlNamespaces = {
     {XmlContentType::ContentTypeXml,    {
        {L"a",               L"http://schemas.openxmlformats.org/package/2006/content-types",                             "AppxPackaging/[Content_Types]/opc-contentTypes.xsd"},
     }},
     {XmlContentType::AppxBlockMapXml,   {
        {L"a",               L"http://schemas.microsoft.com/appx/2010/blockmap",                                          "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd"},
        {L"b",               L"http://schemas.microsoft.com/appx/2015/blockmap",                                          "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd"},
        {L"c",               L"http://schemas.microsoft.com/appx/2017/blockmap",                                          "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd"},
     }},
     {XmlContentType::AppxManifestXml,   {
        {L"win10foundation", L"http://schemas.microsoft.com/appx/manifest/foundation/windows10",                          "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"},
        {L"win10uap",        L"http://schemas.microsoft.com/appx/manifest/uap/windows10",                                 "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"},
        {L"t",               L"http://schemas.microsoft.com/appx/manifest/types",                                         "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"},
        {L"mp",              L"http://schemas.microsoft.com/appx/2014/phone/manifest",                                    "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"},
        {L"foundation2",     L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/2",                        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"},
        {L"uap2",            L"http://schemas.microsoft.com/appx/manifest/uap/windows10/2",                               "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"},        
        {L"uap3",            L"http://schemas.microsoft.com/appx/manifest/uap/windows10/3",                               "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"},
        {L"uap4",            L"http://schemas.microsoft.com/appx/manifest/uap/windows10/4",                               "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"},
        {L"win10wincap",     L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities",      "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"},
        {L"wincap2",         L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/2",    "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd"},
        {L"wincap3",         L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/3",    "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"},
        {L"win10rescap",     L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities",   "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"},
        {L"rescap2",         L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/2", "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"},
        {L"rescap3",         L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/3", "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"},
        {L"rescap4",         L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4", "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"},
        {L"win10iot",        L"http://schemas.microsoft.com/appx/manifest/iot/windows10",                                 "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"},
        {L"iot2",            L"http://schemas.microsoft.com/appx/manifest/iot/windows10/2",                               "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"},
        {L"desktop",         L"http://schemas.microsoft.com/appx/manifest/desktop/windows10",                             "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"},
        {L"desktop2",        L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/2",                           "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"},
        {L"desktop3",        L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/3",                           "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"},
        {L"desktop4",        L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/4",                           "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"},
        {L"com",             L"http://schemas.microsoft.com/appx/manifest/com/windows10",                                 "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"},
        {L"com2",            L"http://schemas.microsoft.com/appx/manifest/com/windows10/2",                               "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"},
        {L"uap5",            L"http://schemas.microsoft.com/appx/manifest/uap/windows10/5",                               "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"},
        {L"uap6",            L"http://schemas.microsoft.com/appx/manifest/uap/windows10/6",                               "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"},
     }}
 };

static std::map<XmlQueryName, std::wstring> xPaths = {
    {XmlQueryName::Package_Identity                             ,L"/*[local-name()='Package']/*[local-name()='Identity']"},
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
    bool    m_clear = true;
public:
    operator VARIANT() && = delete;
    operator VARIANT() & { return m_variant; }
    Variant() : m_clear(true) { VariantInit(&m_variant); }
    Variant(bool value) : m_clear(true)
    {
        VariantInit(&m_variant);
        m_variant.vt = VT_BOOL;
        m_variant.boolVal = value ? VARIANT_TRUE : VARIANT_FALSE;
    }
    Variant(IStream* stream) : m_clear(false)
    {
        m_variant.vt = VT_UNKNOWN;
        m_variant.punkVal = stream;
    }
    Variant(Bstr& value) : m_clear(false)
    {
        m_variant.vt = VT_BSTR;
        m_variant.bstrVal = value.Get();
    }

    template <class T>
    Variant(ComPtr<T>& item) : m_clear(false)
    {
        m_variant.vt = VT_UNKNOWN;
        m_variant.punkVal = item.Get();
    }

    ~Variant() { if (m_clear) {VariantClear(&m_variant);} }

    VARIANT* AddressOf()
    { 
        VariantClear(&m_variant);
        return &m_variant;
    }

    VARIANT& Get() { return m_variant; }
};

class MSXMLElement : public ComClass<MSXMLElement, IXmlElement, IMSXMLElement>
{
    std::pair<bool, Variant> GetAttribute(XmlAttributeName attribute)
    {
        Bstr name(attributeNames[attribute]);
        Variant value;
        ThrowHrIfFailed(m_element->getAttribute(name, value.AddressOf()));
        if (value.Get().vt == VT_BSTR)
        {
            return std::make_pair(true, std::move(value));
        }
        return std::make_pair<bool, Variant>(false, Variant());
    }

public:
    MSXMLElement(ComPtr<IXMLDOMElement>& element) : m_element(element) {}

    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        auto value = GetAttribute(attribute);
        if (value.first)
        {   return utf16_to_utf8(static_cast<WCHAR*>(value.second.Get().bstrVal));
        }
        return "";
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        std::vector<std::uint8_t> result;

        auto intermediate = GetAttributeValue(attribute);;
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

// Because MSXML6 uses IErrorInfo to provide additional details as to why a call could
// fail we'll use a translation-unit-specific macro for logging those additional details.
#define ThrowHrIfFailedWithIErrorInfo(a)                                         \
{   HRESULT result = a;                                                          \
    if (FAILED(result))                                                          \
    {   ComPtr<IErrorInfo> errorInfo;                                            \
        ThrowHrIfFailed(::GetErrorInfo(0, &errorInfo));                          \
        if (nullptr != errorInfo.Get())                                          \
        {   Bstr description;                                                    \
            ThrowHrIfFailed(errorInfo->GetDescription(description.AddressOf())); \
            Global::Log::Append(utf16_to_utf8(description.Get()));               \
        }                                                                        \
        ThrowHrIfFailed(result);                                                 \
    }                                                                            \
}

class MSXMLDom : public ComClass<MSXMLDom, IXmlDom, IMSXMLDom>
{
public:
    MSXMLDom(ComPtr<IStream>& stream, std::vector<std::tuple<std::wstring, std::wstring, std::string>>& namespaces, IMSIXFactory* factory = nullptr)
    {
        ThrowHrIfFailed(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_xmlDocument)));
        ThrowHrIfFailed(m_xmlDocument->put_async(VARIANT_FALSE));

        Variant vTrue(true);
        Bstr property(L"NewParser"); // see https://msdn.microsoft.com/en-us/library/ms767616%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
        ThrowHrIfFailed(m_xmlDocument->setProperty(property.Get(), vTrue.Get()));

        ComPtr<IXMLDOMSchemaCollection2> cache;
        if (factory != nullptr)
        {   ThrowHrIfFailed(CoCreateInstance(__uuidof(XMLSchemaCache60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&cache)));
            ThrowHrIfFailed(cache->put_validateOnLoad(VARIANT_FALSE));
        }

        // process selection namespaces
        std::vector<std::tuple<std::wstring, std::wstring, std::string>> empty;
        std::wostringstream value;
        std::size_t countNamespaces = 0;
        for(auto& item : namespaces)
        {               
            if (0 != countNamespaces++) { value << L" "; } // namespaces are space-delimited
            value << L"xmlns:" << std::get<0>(item) << LR"(=")" << std::get<1>(item) << LR"(")";

            // Process schema XSD for current namespace
            if (factory != nullptr && !(std::get<2>(item).empty()))
            {
                ComPtr<IStream> resource(factory->GetResource(std::get<2>(item)));
                auto schema = ComPtr<IMSXMLDom>::Make<MSXMLDom>(resource, empty)->GetDomDocument();

                long readyState = 0;                
                ThrowHrIfFailed(schema->get_readyState(&readyState));
                ThrowErrorIfNot(Error::Unexpected, (4 == readyState), "The document has not been completely loaded.");

                Bstr schemaNamespace(std::get<1>(item));
                Variant var(schema);
                ThrowHrIfFailedWithIErrorInfo(cache->add(schemaNamespace, var.Get()));
            }
        }

        if (nullptr != cache.Get())
        {   // validate the schema collection and set the schemas for the XML document
            ThrowHrIfFailedWithIErrorInfo(cache->validate());
            Variant var(cache);
            ThrowHrIfFailed(m_xmlDocument->putref_schemas(var.Get()));
        }

        if (!value.str().empty())
        {   // Set selection namespaces for the XML document
            Bstr selectionProperty(L"SelectionNamespaces");
            Bstr selectionValue(value.str());
            Variant var(selectionValue);
            ThrowHrIfFailed(m_xmlDocument->setProperty(selectionProperty, var.Get()));
        }        

        // Now parse the XML document
        Variant var(stream.Get());
        VARIANT_BOOL success(VARIANT_FALSE);
        ThrowHrIfFailed(m_xmlDocument->load(var, &success));
        if (VARIANT_FALSE == success)
        {
            ComPtr<IXMLDOMParseError> error;            
            ThrowHrIfFailed(m_xmlDocument->get_parseError(&error));

            long errorCode = 0, lineNumber = 0, columnNumber = 0;
            ThrowHrIfFailed(error->get_errorCode(&errorCode));
            ThrowHrIfFailed(error->get_line(&lineNumber));
            ThrowHrIfFailed(error->get_linepos(&columnNumber));

            Bstr reason;
            ThrowHrIfFailed(error->get_reason(reason.AddressOf()));
            std::ostringstream message;
            message << "XML error: " << std::hex << errorCode << " on line " << std::dec << lineNumber << ", col " << columnNumber << ".  " << utf16_to_utf8(static_cast<wchar_t*>(reason.Get()));

            // as necessary translate MSXML specific errors w.r.t. malformed XML into generic Xml errors and leave the full details in the log.
            if (errorCode == 0xc00cee65)
            {   errorCode = static_cast<long>(Error::XmlFatal);
            }
            ThrowErrorIf(errorCode, (success == VARIANT_FALSE), message.str());
        }
    }

    // IMSXMLDom
    MSIX::ComPtr<IXMLDOMDocument> GetDomDocument() override
    {
        MSIX::ComPtr<IXMLDOMDocument> result;
        ThrowHrIfFailed(m_xmlDocument->QueryInterface(IID_PPV_ARGS(&result)));
        return result;
    }

    // IXmlDom
    MSIX::ComPtr<IXmlElement> GetDocument() override
    {
        ComPtr<IXMLDOMElement> element;
        ThrowHrIfFailed(m_xmlDocument->get_documentElement(&element));
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
    ComPtr<IXMLDOMDocument2> m_xmlDocument;
};

class MSXMLFactory : public ComClass<MSXMLFactory, IXmlFactory>
{
public:
    MSXMLFactory(IMSIXFactory* factory) : m_factory(factory)
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
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType], m_factory);
            case XmlContentType::AppxManifestXml:
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType], m_factory);
            case XmlContentType::ContentTypeXml:
                return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces[footPrintType], m_factory);
        }
        throw Exception(Error::InvalidParameter);    
    }

protected:
    bool m_CoInitialized = false;
    IMSIXFactory* m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMSIXFactory* factory) { return ComPtr<IXmlFactory>::Make<MSXMLFactory>(factory); }
} // namespace MSIX