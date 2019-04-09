//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include <wchar.h>

#include "Exceptions.hpp"
#include "Log.hpp"
#include "StreamBase.hpp"
#include "IXml.hpp"
#include "Encoding.hpp"
#include "UnicodeConversion.hpp"
#include "MSIXResource.hpp"
#include "Enumerators.hpp"

#include <msxml6.h>

#include "Unknwn.h"
#include "Objidl.h"
// {2730f595-0c80-4f3e-8891-753b2e8c305d}
class IMSXMLElement : public IUnknown
// An internal interface for XML document object model
{
public:
    virtual MSIX::ComPtr<IXMLDOMNodeList> SelectNodes(XmlQueryName query) = 0;
};
MSIX_INTERFACE(IMSXMLElement, 0x2730f595,0x0c80,0x4f3e,0x88,0x91,0x75,0x3b,0x2e,0x8c,0x30,0x5d);

// {b6bca5f0-c6c1-4409-85be-e476aabec19a}
class IMSXMLDom : public IUnknown
// An internal interface for XML document object model
{
public:
    virtual MSIX::ComPtr<IXMLDOMDocument> GetDomDocument() = 0;
};
MSIX_INTERFACE(IMSXMLDom, 0xb6bca5f0,0xc6c1,0x4409,0x85,0xbe,0xe4,0x76,0xaa,0xbe,0xc1,0x9a);

namespace MSIX {

struct SchemaEntry
{
    const wchar_t*  uri;
    const wchar_t*  alias;
    const char*     schema;

    SchemaEntry(const wchar_t* u, const wchar_t* a, const char* s) : uri(u), alias(a), schema(s) {}

    inline bool operator==(const wchar_t* otherUri) const {
        return 0 == wcscmp(uri, otherUri);
    }
};

typedef std::vector<SchemaEntry> NamespaceManager;

//         ALL THE URIs MUST BE LOWER-CASE, ordering of schema entries defines order of placement of schema into schema cache.
static const NamespaceManager xmlNamespaces[] = {
{   // XmlContentType::ContentTypeXml
SchemaEntry(L"http://schemas.openxmlformats.org/package/2006/content-types",                             L"a",               "AppxPackaging/[Content_Types]/opc-contentTypes.xsd")
},
{   // XmlContentType::AppxBlockMapXml
SchemaEntry(L"http://schemas.microsoft.com/appx/2010/blockmap",                                          L"a",               "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2015/blockmap",                                          L"b",               "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2017/blockmap",                                          L"c",               "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd"),
},
{   // XmlContentType::AppxManifestXml
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10",                          L"win10foundation", "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10",                                 L"win10uap",        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/types",                                         L"t",               "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2014/phone/manifest",                                    L"mp",              "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/2",                        L"foundation2",     "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/2",                               L"uap2",            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/3",                               L"uap3",            "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/4",                               L"uap4",            "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities",      L"win10wincap",     "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/2",    L"wincap2",         "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/3",    L"wincap3",         "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities",   L"win10rescap",     "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/2", L"rescap2",         "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/3", L"rescap3",         "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4", L"rescap4",         "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/5", L"rescap5",         "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v5.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/6", L"rescap6",         "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v6.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/mobile/windows10",                              L"win10mobile",     "AppxPackaging/Manifest/Schema/2015/MobileManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/iot/windows10",                                 L"win10iot",        "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/iot/windows10/2",                               L"iot2",            "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/holographic/windows10",                         L"holo",            "AppxPackaging/Manifest/Schema/2015/HolographicManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/serverpreview/windows10",                       L"win10serverpreview", "AppxPackaging/Manifest/Schema/2015/ServerManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10",                             L"desktop",         "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/2",                           L"desktop2",        "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/3",                           L"desktop3",        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/4",                           L"desktop4",        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/5",                           L"desktop5",        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v5.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/desktop/windows10/6",                           L"desktop6",        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v6.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/com/windows10",                                 L"com",             "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/com/windows10/2",                               L"com2",            "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/5",                               L"uap5",            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/6",                               L"uap6",            "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/7",                               L"uap7",            "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v7.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/uap/windows10/8",                               L"uap8",            "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v8.xsd"),
},
{   // XmlContentType::AppxBundleManifestXml
SchemaEntry(L"http://schemas.microsoft.com/appx/manifest/types",                                         L"t",               "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2013/bundle",                                            L"b",               "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2016/bundle",                                            L"b2",              "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2017/bundle",                                            L"b3",              "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd"),
SchemaEntry(L"http://schemas.microsoft.com/appx/2018/bundle",                                            L"b4",              "AppxPackaging/Manifest/Schema/2018/BundleManifestSchema2018.xsd"),
}};

// --------------------------------------------------------
// MSXML6 specific error codes
// --------------------------------------------------------
// MSG_E_UNDECLAREDPREFIX              - Undeclared prefix
#define UNDECLAREDPREFIX        0xc00cee65
// XMLOM_VALIDATE_DECLARATION_NOTFOUND - The node is neither valid nor invalid because no DTD/Schema declaration was found.
#define DECLARATION_NOTFOUND    0xc00ce224
// XML_EMPTY_NOT_ALLOWED               - Element cannot be empty according to the DTD/Schema.
#define ELEMENT_EMPTY           0xc00ce011
// XML_INVALID_CONTENT                 - Element content is invalid according to the DTD/Schema.
#define INVALID_CONTENT         0xc00ce014

class Bstr
{
    BSTR m_bstr;
public:
    operator BSTR() && = delete;
    operator BSTR() & { return m_bstr; }
    Bstr() {m_bstr = nullptr;}
    Bstr(std::wstring text)
    {
        m_bstr = ::SysAllocStringLen(text.c_str(), static_cast<UINT>(text.length())); 
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
    Variant(const Variant&) = delete;
    Variant(Variant&& other) : m_clear(other.m_clear)
    {
        other.m_clear = false;
        std::swap(m_variant, other.m_variant);
    }
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
    Variant(const ComPtr<T>& item) : m_clear(false)
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

class MSXMLElement final : public ComClass<MSXMLElement, IXmlElement, IMSXMLElement, IMsixElement>
{
public:
    MSXMLElement(IMsixFactory* factory, ComPtr<IXMLDOMElement>& element) : m_factory(factory), m_element(element) {}

    // IXmlElement
    std::string GetAttributeValue(XmlAttributeName attribute) override
    {
        Variant value;
        if (GetAttribute(attributeNames[static_cast<std::uint8_t>(attribute)], value.AddressOf()))
        {
            return wstring_to_utf8(static_cast<WCHAR*>(value.Get().bstrVal));
        }
        return "";
    }

    std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) override
    {
        auto intermediate = GetAttributeValue(attribute);
        return Encoding::GetBase64DecodedValue(intermediate);
    }

    std::string GetText() override
    {
        ComPtr<IXMLDOMNode> node;
        ThrowHrIfFailed(m_element->QueryInterface(__uuidof(IXMLDOMNode), reinterpret_cast<void**>(&node)));
        Bstr value;
        ThrowHrIfFailed(node->get_text(value.AddressOf()));
        if (value.Get() != nullptr)
        {
            return wstring_to_utf8(static_cast<WCHAR*>(value.Get()));
        }
        return "";
    }

    // IMSXMLElement
    ComPtr<IXMLDOMNodeList> SelectNodes(XmlQueryName query) override
    {
        Bstr xPath(xPaths[static_cast<std::uint8_t>(query)]);
        ComPtr<IXMLDOMNodeList> list;
        ThrowHrIfFailed(m_element->selectNodes(xPath, &list));
        return list;
    }

    // IMsixElement
    HRESULT STDMETHODCALLTYPE GetAttributeValue(LPCWSTR name, LPWSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        *value = nullptr;
        Variant attribute;
        if (GetAttribute(name, attribute.AddressOf()))
        {
            auto intermediate = std::wstring(static_cast<WCHAR*>(attribute.Get().bstrVal));
            ThrowHrIfFailed(m_factory->MarshalOutWstring(intermediate, value));
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetText(LPWSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        ComPtr<IXMLDOMNode> node;
        ThrowHrIfFailed(m_element->QueryInterface(__uuidof(IXMLDOMNode), reinterpret_cast<void**>(&node)));
        Bstr text;
        ThrowHrIfFailed(node->get_text(text.AddressOf()));
        if (text.Get() != nullptr)
        {
            auto intermediate = std::wstring(static_cast<WCHAR*>(text.Get()));
            ThrowHrIfFailed(m_factory->MarshalOutWstring(intermediate, value));
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetElements(LPCWSTR name, IMsixElementEnumerator** elements) noexcept override try
    {
        ComPtr<IXMLDOMNodeList> list;
        Bstr xPath(name);
        ThrowHrIfFailed(m_element->selectNodes(xPath, &list));

        long count = 0;
        ThrowHrIfFailed(list->get_length(&count));
        std::vector<ComPtr<IMsixElement>> elementsEnum;
        for(long index=0; index < count; index++)
        {
            ComPtr<IXMLDOMNode> node;
            ThrowHrIfFailed(list->get_item(index, &node));
            ComPtr<IXMLDOMElement> elementItem;
            ThrowHrIfFailed(node->QueryInterface(__uuidof(IXMLDOMElement), reinterpret_cast<void**>(&elementItem)));
            auto item = ComPtr<IMsixElement>::Make<MSXMLElement>(m_factory, elementItem);
            elementsEnum.push_back(std::move(item));
        }
        *elements = ComPtr<IMsixElementEnumerator>::
            Make<EnumeratorCom<IMsixElementEnumerator,IMsixElement>>(elementsEnum).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetAttributeValueUtf8(LPCSTR name, LPSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        *value = nullptr;
        Variant attribute;
        auto wname = utf8_to_wstring(name);
        if (GetAttribute(wname, attribute.AddressOf()))
        {
            auto intermediate = wstring_to_utf8(std::wstring(static_cast<WCHAR*>(attribute.Get().bstrVal)));
            ThrowHrIfFailed(m_factory->MarshalOutStringUtf8(intermediate, value));
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetTextUtf8(LPSTR* value) noexcept override try
    {
        ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer.");
        ComPtr<IXMLDOMNode> node;
        ThrowHrIfFailed(m_element->QueryInterface(__uuidof(IXMLDOMNode), reinterpret_cast<void**>(&node)));
        Bstr text;
        ThrowHrIfFailed(node->get_text(text.AddressOf()));
        if (text.Get() != nullptr)
        {
            auto intermediate = wstring_to_utf8(std::wstring(static_cast<WCHAR*>(text.Get())));
            ThrowHrIfFailed(m_factory->MarshalOutStringUtf8(intermediate, value));
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE GetElementsUtf8(LPCSTR name, IMsixElementEnumerator** elements) noexcept override try
    {
        return GetElements(utf8_to_wstring(name).c_str(), elements);
    } CATCH_RETURN();

protected:

    bool GetAttribute(const std::wstring& attribute, VARIANT* variant)
    {
        Bstr name(attribute);
        ThrowHrIfFailed(m_element->getAttribute(name, variant));
        return (variant->vt == VT_BSTR);
    }

    ComPtr<IMsixElementEnumerator> GetElementsHelper(std::wstring& name)
    {
        ComPtr<IXMLDOMNodeList> list;
        Bstr xPath(name);
        ThrowHrIfFailed(m_element->selectNodes(xPath, &list));

        long count = 0;
        ThrowHrIfFailed(list->get_length(&count));
        std::vector<ComPtr<IMsixElement>> elementsEnum;
        for(long index=0; index < count; index++)
        {
            ComPtr<IXMLDOMNode> node;
            ThrowHrIfFailed(list->get_item(index, &node));
            ComPtr<IXMLDOMElement> elementItem;
            ThrowHrIfFailed(node->QueryInterface(__uuidof(IXMLDOMElement), reinterpret_cast<void**>(&elementItem)));
            auto item = ComPtr<IMsixElement>::Make<MSXMLElement>(m_factory, elementItem);
            elementsEnum.push_back(std::move(item));
        }
        return ComPtr<IMsixElementEnumerator>::Make<EnumeratorCom<IMsixElementEnumerator,IMsixElement>>(elementsEnum);
    }

    IMsixFactory* m_factory;
    ComPtr<IXMLDOMElement> m_element;
};

// Because MSXML6 uses IErrorInfo to provide additional details as to why a call could
// fail we'll use a translation-unit-specific macro for logging those additional details.
#define ThrowHrIfFailedWithIErrorInfo(a)                                         \
{   HRESULT result = a;                                                          \
    if (FAILED(result))                                                          \
    {   ComPtr<IErrorInfo> errorInfo;                                            \
        ThrowHrIfFailed(::GetErrorInfo(0, &errorInfo));                          \
        if (errorInfo)                                                           \
        {   Bstr description;                                                    \
            ThrowHrIfFailed(errorInfo->GetDescription(description.AddressOf())); \
            Global::Log::Append(wstring_to_utf8(description.Get()));               \
        }                                                                        \
        ThrowHrIfFailed(result);                                                 \
    }                                                                            \
}

class MSXMLDom final : public ComClass<MSXMLDom, IXmlDom, IMSXMLDom>
{
public:
    MSXMLDom(const ComPtr<IStream>& stream, const NamespaceManager& namespaces, IMsixFactory* factory = nullptr, bool stripIgnorableNamespaces = false) : m_factory(factory)
    {
        ThrowHrIfFailed(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_xmlDocument)));
        ThrowHrIfFailed(m_xmlDocument->put_async(VARIANT_FALSE));

        Variant vTrue(true);
        Bstr property(L"NewParser"); // see https://msdn.microsoft.com/en-us/library/ms767616%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
        ThrowHrIfFailed(m_xmlDocument->setProperty(property.Get(), vTrue.Get()));

        if (nullptr != m_factory && !namespaces.empty())
        {   // Create and populate schema cache
            ComPtr<IXMLDOMSchemaCollection2> cache;       
            ThrowHrIfFailed(CoCreateInstance(__uuidof(XMLSchemaCache60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&cache)));
            ThrowHrIfFailed(cache->put_validateOnLoad(VARIANT_FALSE));

            std::size_t countNamespaces = 0; 
            std::wostringstream value;
            for(const auto& item : namespaces)
            {
                std::wstring uri = item.uri;
                if (0 != countNamespaces++) { value << L" "; } // namespaces are space-delimited
                value << L"xmlns:" << item.alias << LR"(=")" << uri << LR"(")";
                // Process schema XSD for current namespace
                NamespaceManager emptyManager;                
                ComPtr<IStream> resource(m_factory->GetResource(item.schema));
                auto schema = ComPtr<IMSXMLDom>::Make<MSXMLDom>(resource, emptyManager)->GetDomDocument();

                long readyState = 0;                
                ThrowHrIfFailed(schema->get_readyState(&readyState));
                ThrowErrorIfNot(Error::Unexpected, (4 == readyState), "The document has not been completely loaded.");

                Bstr schemaNamespace(uri);
                Variant var(schema);
                ThrowHrIfFailedWithIErrorInfo(cache->add(schemaNamespace, var.Get()));
            }
            // Set selection namespaces for the XML document
            Bstr selectionProperty(L"SelectionNamespaces");
            Bstr selectionValue(value.str());
            Variant selectionVariant(selectionValue);
            ThrowHrIfFailed(m_xmlDocument->setProperty(selectionProperty, selectionVariant.Get()));
            // Validate the schema collection and set the schemas for the XML document
            ThrowHrIfFailedWithIErrorInfo(cache->validate());
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
            hash validation failure, for instance) will result in an error other than SignatureInvalid
            being returned to the caller.  To prevent that, we will copy the stream into a memory buffer
            (so that should read fail, it does so there with the correct error), and then create our DOM
            with deferred validation from the in memory copy; but first we need to compute the size of
            the buffer needed...
            */
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            ULARGE_INTEGER bytesCount = {0};
            bytesCount.QuadPart = end.u.LowPart;
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
                    ThrowErrorIfNot(Error::XmlError, attribute, "ignorable namespace alias is not actually defined as valid xml namespace." );
                    Variant attributeValue;
                    ThrowHrIfFailed(attribute->get_nodeValue(attributeValue.AddressOf()));
                    ThrowErrorIf(Error::XmlError, (VT_NULL == attributeValue.Get().vt), "ignorable namespace alias has empty target namespace URI.");
                    std::wstring uri(reinterpret_cast<WCHAR*>(attributeValue.Get().bstrVal));
                    std::transform(uri.begin(), uri.end(), uri.begin(), ::tolower);
                    // next we look for that uri against our namespace manager                    
                    const auto& result = std::find(namespaces.begin(), namespaces.end(), uri.c_str());
                    if (result == namespaces.end())
                    {   // the namespace specified is unknown to us.  remove everything with the specified alias!
                        std::wostringstream xPath;
                        xPath << L"//*[namespace-uri()='"       << reinterpret_cast<WCHAR*>(attributeValue.Get().bstrVal) 
                              << L"' or //@*[namespace-uri()='" << reinterpret_cast<WCHAR*>(attributeValue.Get().bstrVal) 
                              << L"']]";
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
        // Because we don't create shallow copies of Streams, it is important 
        // to reset the stream back to the beginning after reading it.
        LARGE_INTEGER li = {0};
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));

        ComPtr<IXMLDOMParseError> error;        
        if (VARIANT_FALSE == success) { ThrowHrIfFailed(m_xmlDocument->get_parseError(&error)); }
        if (stripIgnorableNamespaces && nullptr == error.Get())
        {
            long readyState = 0;                
            ThrowHrIfFailed(m_xmlDocument->get_readyState(&readyState));
            ThrowErrorIfNot(Error::Unexpected, (4 == readyState), "The document has not been completely loaded.");
            ThrowHrIfFailed(m_xmlDocument->validate(&error));
        }

        if(error)
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
                if (nullptr != reason.Get()) { message << "  " << wstring_to_utf8(static_cast<wchar_t*>(reason.Get())); }

                // As necessary, translate MSXML6-specific errors w.r.t. malformed/non-schema-compliant 
                // XML into generic Xml errors and leave the full details in the log.
                if (UNDECLAREDPREFIX == errorCode || DECLARATION_NOTFOUND == errorCode)
                {   // file is either invalid XML, or it's valid, but no schema was found for it.
                    errorCode = static_cast<long>(Error::XmlFatal);
                }
                else if (ELEMENT_EMPTY == errorCode || INVALID_CONTENT == errorCode)
                {   // file is valid XML, but it failed according to the schema provided.
                    errorCode = static_cast<long>(Error::XmlError);
                }
                ThrowErrorIf(errorCode, (true), message.str().c_str());      
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
        return ComPtr<IXmlElement>::Make<MSXMLElement>(m_factory, element);
    }
    
    bool ForEachElementIn(const ComPtr<IXmlElement>& root, XmlQueryName query, XmlVisitor& visitor) override
    {
        ComPtr<IMSXMLElement> element = root.As<IMSXMLElement>();
        ComPtr<IXMLDOMNodeList> list = element->SelectNodes(query);

        long count = 0;
        ThrowHrIfFailed(list->get_length(&count));
        for(long index=0; index < count; index++)
        {
            ComPtr<IXMLDOMNode> node;
            ThrowHrIfFailed(list->get_item(index, &node));
            ComPtr<IXMLDOMElement> elementItem;
            ThrowHrIfFailed(node->QueryInterface(__uuidof(IXMLDOMElement), reinterpret_cast<void**>(&elementItem)));
            auto item = ComPtr<IXmlElement>::Make<MSXMLElement>(m_factory, elementItem);
            if (!visitor.Callback(visitor.context, item))
            {
                return false;
            }
        }
        return true;
    }

protected:
    ComPtr<IXMLDOMDocument2> m_xmlDocument;
    IMsixFactory* m_factory;
};

class MSXMLFactory final : public ComClass<MSXMLFactory, IXmlFactory>
{
public:
    MSXMLFactory(IMsixFactory* factory) : m_factory(factory), m_CoInitialized(false)
    {
        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        m_CoInitialized = SUCCEEDED(result);
        if (RPC_E_CHANGED_MODE == result) { result = S_FALSE; }
        ThrowHrIfFailed(result);
    }

    ~MSXMLFactory() { if (m_CoInitialized) { CoUninitialize(); m_CoInitialized = false; } }    

    ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const ComPtr<IStream>& stream) override
    {   
        #if VALIDATING
        bool HasIgnorableNamespaces = (XmlContentType::AppxManifestXml == footPrintType);
        #else
        bool HasIgnorableNamespaces = false;
        NamespaceManager emptyManager; 
        #endif

        return ComPtr<IXmlDom>::Make<MSXMLDom>(
            stream, 
            #if VALIDATING        
                xmlNamespaces[static_cast<std::uint8_t>(footPrintType)],
            #else
                emptyManager,
            #endif
            m_factory,
            HasIgnorableNamespaces);
    }

protected:
    bool            m_CoInitialized;
    IMsixFactory*   m_factory;
};

ComPtr<IXmlFactory> CreateXmlFactory(IMsixFactory* factory) { return ComPtr<IXmlFactory>::Make<MSXMLFactory>(factory); }
} // namespace MSIX