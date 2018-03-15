#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>

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
typedef std::map<std::wstring, std::tuple<std::wstring, std::string>>   NamespaceManager;
typedef std::map<XmlContentType, NamespaceManager>                      NamespaceTable;

//              content type -> [map uri <-> [alias,XSD]]
//         ALL THE URIs MUST BE LOWER-CASE
static const NamespaceTable xmlNamespaces = {
     {XmlContentType::ContentTypeXml,    {
        {L"http://schemas.openxmlformats.org/package/2006/content-types",                             {L"a",               "AppxPackaging/[Content_Types]/opc-contentTypes.xsd"}},
     }},
     {XmlContentType::AppxBlockMapXml,   {
        {L"http://schemas.microsoft.com/appx/2010/blockmap",                                          {L"a",               "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/2015/blockmap",                                          {L"b",               "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd"}},
        {L"http://schemas.microsoft.com/appx/2017/blockmap",                                          {L"c",               "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd"}},
     }},
     {XmlContentType::AppxManifestXml,   {
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10",                          {L"win10foundation", "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10",                                 {L"win10uap",        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/types",                                         {L"t",               "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"}},
        {L"http://schemas.microsoft.com/appx/2014/phone/manifest",                                    {L"mp",              "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/2",                        {L"foundation2",     "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10/2",                               {L"uap2",            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"}}, 
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10/3",                               {L"uap3",            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10/4",                               {L"uap4",            "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities",      {L"win10wincap",     "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/2",    {L"wincap2",         "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/3",    {L"wincap3",         "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities",   {L"win10rescap",     "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/2", {L"rescap2",         "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/3", {L"rescap3",         "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4", {L"rescap4",         "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/iot/windows10",                                 {L"win10iot",        "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/iot/windows10/2",                               {L"iot2",            "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/desktop/windows10",                             {L"desktop",         "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/2",                           {L"desktop2",        "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/3",                           {L"desktop3",        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/4",                           {L"desktop4",        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/com/windows10",                                 {L"com",             "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/com/windows10/2",                               {L"com2",            "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10/5",                               {L"uap5",            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"}},
        {L"http://schemas.microsoft.com/appx/manifest/uap/windows10/6",                               {L"uap6",            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"}},
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
    MSXMLDom(ComPtr<IStream>& stream, const NamespaceManager& namespaces, IMSIXFactory* factory = nullptr, bool stripIgnorableNamespaces = false)
    {
        ThrowHrIfFailed(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_xmlDocument)));
        ThrowHrIfFailed(m_xmlDocument->put_async(VARIANT_FALSE));

        Variant vTrue(true);
        Bstr property(L"NewParser"); // see https://msdn.microsoft.com/en-us/library/ms767616%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
        ThrowHrIfFailed(m_xmlDocument->setProperty(property.Get(), vTrue.Get()));

        if (!namespaces.empty())
        {   // Create and populate schema cache
            ComPtr<IXMLDOMSchemaCollection2> cache;       
            ThrowHrIfFailed(CoCreateInstance(__uuidof(XMLSchemaCache60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&cache)));
            ThrowHrIfFailed(cache->put_validateOnLoad(VARIANT_FALSE));

            std::size_t countNamespaces = 0; 
            std::wostringstream value;
            for(const auto& item : namespaces)
            {
                if (0 != countNamespaces++) { value << L" "; } // namespaces are space-delimited
                value << L"xmlns:" << std::get<0>(item.second) << LR"(=")" << item.first << LR"(")";
                // Process schema XSD for current namespace
                NamespaceManager emptyManager;                
                ComPtr<IStream> resource(factory->GetResource(std::get<1>(item.second)));
                auto schema = ComPtr<IMSXMLDom>::Make<MSXMLDom>(resource, emptyManager)->GetDomDocument();

                long readyState = 0;                
                ThrowHrIfFailed(schema->get_readyState(&readyState));
                ThrowErrorIfNot(Error::Unexpected, (4 == readyState), "The document has not been completely loaded.");

                Bstr schemaNamespace(item.first);
                Variant var(schema);
                ThrowHrIfFailedWithIErrorInfo(cache->add(schemaNamespace, var.Get()));
            }
            // Set selection namespaces for the XML document
            Bstr selectionProperty(L"SelectionNamespaces");
            Bstr selectionValue(value.str());
            Variant selectionVariant(selectionValue);
            ThrowHrIfFailed(m_xmlDocument->setProperty(selectionProperty, selectionVariant.Get()));
            ThrowHrIfFailedWithIErrorInfo(cache->validate());
            // If we're not going to strip namespaces, then just load the stream that's passed in.
            // Validate the schema collection and set the schemas for the XML document
            Variant schemasVariant(cache);
            ThrowHrIfFailed(m_xmlDocument->putref_schemas(schemasVariant.Get()));            
        }               

        // Now parse the XML document            
        VARIANT_BOOL success(VARIANT_FALSE);        
        if (!stripIgnorableNamespaces)
        {   
            Variant var(stream);
            ThrowHrIfFailed(m_xmlDocument->load(var, &success));        
        }
        else
        {   /* Because load will only parse, a failure to read the stream (e.g. due to a blockmap 
            hash validation failuire, for instance) will result in an error other than SignatureInvalid
            being returned to the caller.  To prevent that, we will copy the stream into a memory buffer
            (so that should read fail, it does so there with the correct error), and then create our DOM
            with deferred validation from the in memory copy; but first we need to compute the size of
            the buffer needed...
            */
            auto file = stream.As<IAppxFile>();
            UINT64 size = 0;
            ThrowHrIfFailed(file->GetSize(&size));
            ULARGE_INTEGER bytesCount = {0};
            bytesCount.QuadPart = size;
            // Now create the in memory copy
            ComPtr<IStream> inMemoryCopy;    
            ThrowHrIfFailed(CreateStreamOnHGlobal(NULL, TRUE, &inMemoryCopy));                        
            ThrowHrIfFailed(inMemoryCopy->SetSize(bytesCount));
            ThrowHrIfFailed(stream->CopyTo(inMemoryCopy.Get(), bytesCount, nullptr, nullptr));
            // It is important to reset the stream back to the beginning!
            LARGE_INTEGER li = {0};
            ThrowHrIfFailed(inMemoryCopy->Seek(li, StreamBase::Reference::START, nullptr));
            Variant var(inMemoryCopy);
            // Now tell the DOM to not validate (as we'll do that later), but instead only parse our input
            ThrowHrIfFailed(m_xmlDocument->put_validateOnParse(VARIANT_FALSE));            
            ThrowHrIfFailed(m_xmlDocument->load(var, &success));
                        
            if (VARIANT_TRUE == success)
            {   // Now that the DOM has been parsed, we can go about stripping ignorable namespaces
                ComPtr<IXMLDOMElement> element;
                ThrowHrIfFailed(m_xmlDocument->get_documentElement(&element));
                ComPtr<IXMLDOMNodeList> namespaceAliasesToStrip;
                Bstr ignorableNamespaces(L"@IgnorableNamespaces");
                ThrowHrIfFailed(element->selectNodes(ignorableNamespaces, &namespaceAliasesToStrip));
                long count = 0;
                ThrowHrIfFailed(namespaceAliasesToStrip->get_length(&count));
                ThrowErrorIf(Error::XmlError, (count > 1), "Only one IgnorableNamespaces attribute allowed");

                std::vector<std::wstring> aliasesToLookup;
                for(long index=0; index < count; index++)
                {   // get the list of ignorable namespace aliases
                    ComPtr<IXMLDOMNode> node;
                    ThrowHrIfFailed(namespaceAliasesToStrip->get_item(index, &node));
                    ComPtr<IXMLDOMAttribute> attribute;
                    ThrowHrIfFailed(node->QueryInterface(__uuidof(IXMLDOMAttribute), reinterpret_cast<void**>(&attribute)));
                    Variant attributeValue;
                    ThrowHrIfFailed(attribute->get_nodeValue(attributeValue.AddressOf()));
                    if (attributeValue.Get().vt != VT_NULL)
                    {
                        std::wistringstream aliases(attributeValue.Get().bstrVal);
                        std::wstring alias;
                        while(getline(aliases, alias, L' ')) { aliasesToLookup.push_back(alias); }
                    }
                }

                // Now look-up each alias to find the URI, and check that URI against our namespace manager
                // if we find the URI in our namespace manager, we don't need to remove any node from the
                // namespace.  However, if we do NOT find the URI in our manager, then we need to remove every
                // node in that namespace.
                ComPtr<IXMLDOMNamedNodeMap> attributes;
                ThrowHrIfFailed(element->get_attributes(&attributes));
                for (const auto& alias : aliasesToLookup)
                {   // first we have to look-up the alias by its xmlns:[alias] attribute name.
                    std::wostringstream builder;
                    builder << L"xmlns:" << alias;
                    Bstr namespaceAlias(builder.str());
                    ComPtr<IXMLDOMNode> attribute;
                    ThrowHrIfFailed(attributes->getNamedItem(namespaceAlias, &attribute));
                    ThrowErrorIf(Error::XmlError, (nullptr == attribute.Get()), "ignorable namespace alias is not actually defined as valid xml namespace." );
                    Variant attributeValue;
                    ThrowHrIfFailed(attribute->get_nodeValue(attributeValue.AddressOf()));
                    ThrowErrorIf(Error::XmlError, (VT_NULL == attributeValue.Get().vt), "ignorable namespace alias has empty target namespace URI.");
                    std::wstring uri(reinterpret_cast<WCHAR*>(attributeValue.Get().bstrVal));
                    std::transform(uri.begin(), uri.end(), uri.begin(), ::tolower);
                    // next we look for that uri against our namespace manager
                    const auto& result = namespaces.find(uri);
                    if (result == namespaces.end())
                    {   // the namespace specified is unknown to us.  remove everything with the specified alias!
                        std::wostringstream xPath;
                        xPath << L"//*[namespace-uri()='" << uri << "']";
                        Bstr query(xPath.str());
                        ComPtr<IXMLDOMNodeList> ignorableNodes;
                        ThrowHrIfFailed(element->selectNodes(query, &ignorableNodes));
                        ComPtr<IXMLDOMSelection> selection;
                        ThrowHrIfFailed(ignorableNodes->QueryInterface(__uuidof(IXMLDOMSelection), reinterpret_cast<void**>(&selection)));
                        ThrowHrIfFailed(selection->removeAll());
                    }
                }       
            }
        }        

        ComPtr<IXMLDOMParseError> error;        
        if (VARIANT_FALSE == success) { ThrowHrIfFailed(m_xmlDocument->get_parseError(&error)); }
        if (stripIgnorableNamespaces && nullptr == error.Get())
        {
            long readyState = 0;                
            ThrowHrIfFailed(m_xmlDocument->get_readyState(&readyState));
            ThrowErrorIfNot(Error::Unexpected, (4 == readyState), "The document has not been completely loaded.");
            ThrowHrIfFailed(m_xmlDocument->validate(&error));
        }

        if(nullptr != error.Get())
        {
            long errorCode = 0, lineNumber = 0, columnNumber = 0;
            ThrowHrIfFailed(error->get_errorCode(&errorCode));
            if (0 != errorCode)
            {
                ThrowHrIfFailed(error->get_line(&lineNumber));
                ThrowHrIfFailed(error->get_linepos(&columnNumber));

                Bstr reason;
                ThrowHrIfFailed(error->get_reason(reason.AddressOf()));
                std::ostringstream message;            
                message << "XML error: " << std::hex << errorCode << " on line " << std::dec << lineNumber << ", col " << columnNumber << ".";
                if (nullptr != reason.Get()) { message << "  " << utf16_to_utf8(static_cast<wchar_t*>(reason.Get())); }

                // As necessary, translate MSXML6-specific errors w.r.t. malformed/non-schema-compliant 
                // XML into generic Xml errors and leave the full details in the log.
                if (0xc00cee65 == errorCode || 0xc00ce224  == errorCode)
                {   // file is either invalid XML, or it's valid, but failed schema validation.
                    errorCode = static_cast<long>(Error::XmlFatal);
                }
                ThrowErrorIf(errorCode, (true), message.str());      
            }      
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
    MSXMLFactory(IMSIXFactory* factory) : m_factory(factory), m_CoInitialized(false)
    {
        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        m_CoInitialized = SUCCEEDED(result);
        if (RPC_E_CHANGED_MODE == result) { result = S_FALSE; }
        ThrowHrIfFailed(result);
    }

    ~MSXMLFactory() { if (m_CoInitialized) { CoUninitialize(); m_CoInitialized = false; } }    

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, ComPtr<IStream>& stream) override
    {   
        bool HasIgnorableNamespaces = (XmlContentType::AppxManifestXml == footPrintType);
        return ComPtr<IXmlDom>::Make<MSXMLDom>(stream, xmlNamespaces.at(footPrintType), m_factory, HasIgnorableNamespaces);
    }

protected:
    bool            m_CoInitialized;
    IMSIXFactory*   m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMSIXFactory* factory) { return ComPtr<IXmlFactory>::Make<MSXMLFactory>(factory); }
} // namespace MSIX