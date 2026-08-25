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
    ContentTypeXml,
    AppxBlockMapXml,
    AppxManifestXml,
    AppxBundleManifestXml,
};

// TODO: Strongly consider creating a more flexible/dynamic way of query construction, rather than this hardcoding
// defines queries for use in IXmlDom::ForEachElementIn
enum class XmlQueryName : std::uint8_t
{
    Package_Identity,
    BlockMap_File,
    Child_Block,
    Bundle_Identity,
    Bundle_Packages_Package,
    Child_Resources_Resource,
    Package_Dependencies_TargetDeviceFamily,
    Package_Applications_Application,
    Package_Properties,
    Child_Description,
    Child_DisplayName,
    Child_PublisherDisplayName,
    Child_Logo,
    Child_Framework,
    Child_ResourcePackage,
    Child_AllowExecution,
    Package_Dependencies_PackageDependency,
    Package_Capabilities_Capability,
    Package_Resources_Resource,
    Child_Identity,
    Package_Dependencies_MainPackageDependency,
    Package_Applications,
    Package_Capabilities,
    Package_Extensions,
    Package_Properties_Framework,
    Package_Properties_ResourcePackage,
    Package_Properties_SupportedUsers,
    Package_Capabilities_CustomCapability,
};

// defines attribute names for use in IXmlElement:: [GetAttributeValue|GetBase64DecodedAttributeValue]
enum class XmlAttributeName : std::uint8_t
{
    Name,
    ResourceId,
    Version,
    Size,
    Identity_ProcessorArchitecture,
    Publisher,
    BlockMap_File_LocalFileHeaderSize,
    BlockMap_File_Block_Hash,
    Bundle_Package_FileName,
    Bundle_Package_Offset,
    Bundle_Package_Type,
    Bundle_Package_Architecture,
    Language,
    MinVersion,
    Dependencies_Tdf_MaxVersionTested,
    Scale,
    Package_Applications_Application_Id,
    Category,
    MaxMajorVersionTested,
    DXFeatureLevel,
};

// {ac94449e-442d-4bed-8fca-83770c0f7ee9}
#ifndef WIN32
interface IXmlElement : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IXmlElement : public IUnknown
#endif
// An internal interface for XML elements
{
public:
    virtual std::string               GetAttributeValue(XmlAttributeName attribute) = 0;
    virtual std::vector<std::uint8_t> GetBase64DecodedAttributeValue(XmlAttributeName attribute) = 0;
    virtual std::string               GetText() = 0;
    virtual std::string               GetPrefix() = 0;
};
MSIX_INTERFACE(IXmlElement, 0xac94449e,0x442d,0x4bed,0x8f,0xca,0x83,0x77,0x0c,0x0f,0x7e,0xe9);

struct XmlVisitor
{
public:
    typedef bool(*lambda)(void*, const MSIX::ComPtr<IXmlElement>&);
    typedef bool(*noContextLambda)(const MSIX::ComPtr<IXmlElement>&);

private:
    void* context = nullptr;
    lambda Callback = nullptr;

    static bool NoContextCallbackImpl(void* context, const MSIX::ComPtr<IXmlElement>& element)
    {
        return reinterpret_cast<noContextLambda>(context)(element);
    }

public:
    // Allow for const data to be passed in, but strip the const.
    XmlVisitor(const void* c, lambda f) : context(const_cast<void*>(c)), Callback(f) {}

    // Allow a lambda that doesn't take a context to be more easily implemented.
    XmlVisitor(noContextLambda f) : context(reinterpret_cast<void*>(f)), Callback(NoContextCallbackImpl) {}

    bool operator()(const MSIX::ComPtr<IXmlElement>& element) { return Callback(context, element); }
};

// TODO: Evaluate impact to binary size.
// Type erased XmlVisitor, allowing for capture based lambdas to pass to ForEachElementIn.
//struct XmlVisitor
//{
//private:
//    struct XmlVisitorConcept
//    {
//        virtual bool operator()(const MSIX::ComPtr<IXmlElement>& element) = 0;
//    };
//
//    template <typename L>
//    struct XmlVisitorModel : public XmlVisitorConcept
//    {
//        XmlVisitorModel(L& l) : lambda(l) {}
//
//        bool operator()(const MSIX::ComPtr<IXmlElement>& element) override { return lambda(element); }
//
//    private:
//        L lambda;
//    };
//
//    std::unique_ptr<XmlVisitorConcept> visitor;
//
//public:
//    template <typename L>
//    XmlVisitor(L& l)
//    {
//        visitor = std::make_unique<XmlVisitorModel<L>>(l);
//    }
//
//    bool operator()(const MSIX::ComPtr<IXmlElement>& element) { return (*visitor)(element); }
//};

// {0e7a446e-baf7-44c1-b38a-216bfa18a1a8}
#ifndef WIN32
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

    bool ForEachElementIn(XmlQueryName query, XmlVisitor& visitor)
    {
        return ForEachElementIn(GetDocument(), query, visitor);
    }
};
MSIX_INTERFACE(IXmlDom, 0x0e7a446e,0xbaf7,0x44c1,0xb3,0x8a,0x21,0x6b,0xfa,0x18,0xa1,0xa8);

// {f82a60ec-fbfc-4cb9-bc04-1a0fe2b4d5be}
#ifndef WIN32
interface IXmlFactory : public IUnknown
#else
class IXmlFactory : public IUnknown
#endif
// An internal interface for creating an IXmlDom object as well as managing XML services lifetime
{
public:
    virtual MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const MSIX::ComPtr<IStream>& stream) = 0;
};
MSIX_INTERFACE(IXmlFactory, 0xf82a60ec,0xfbfc,0x4cb9,0xbc,0x04,0x1a,0x0f,0xe2,0xb4,0xd5,0xbe);

namespace MSIX {
    MSIX::ComPtr<IXmlFactory> CreateXmlFactory(IMsixFactory* factory);

    template <typename T>
    struct StringToNumber
    {
        static uint32_t Get(const std::string&)
        {
            static_assert(False<T>::value, "An appropriate specialization must be specified");
            return 0;   // silence GCC warnings
        }
    };

    template <>
    struct StringToNumber<std::uint32_t>
    {
        static uint32_t Get(const std::string& str)
        {
            return static_cast<uint32_t>(std::stoul(str));
        }
    };

    template <>
    struct StringToNumber<std::uint64_t>
    {
        static uint64_t Get(const std::string& str)
        {
            return static_cast<uint64_t>(std::stoull(str));
        }
    };

    template <class T>
    static T GetNumber(const ComPtr<IXmlElement>& element, XmlAttributeName attribute, T defaultValue)
    {
        const auto& attributeValue = element->GetAttributeValue(attribute);
        bool hasValue = !attributeValue.empty();
        T value = defaultValue;
        if (hasValue)
        {
            try
            {
                value = StringToNumber<T>::Get(attributeValue);
            }
            catch (std::invalid_argument& ia)
            {
                ThrowErrorAndLog(Error::XmlInvalidData, ia.what());
            }
            catch (std::out_of_range& oor)
            {
                ThrowErrorAndLog(Error::XmlInvalidData, oor.what());
            }
            catch (...)
            {
                ThrowErrorAndLog(Error::XmlInvalidData, "Unexpected exception converting string to number");
            }
        }
        return value;
    }

#ifdef USING_MSXML
    using XmlQueryNameCharType = wchar_t;
#else
    using XmlQueryNameCharType = char;
#endif

    const XmlQueryNameCharType* GetQueryString(XmlQueryName query);
    std::string GetQueryStringUtf8(XmlQueryName query);

    std::wstring GetAttributeNameString(XmlAttributeName attr);
    const char* GetAttributeNameStringUtf8(XmlAttributeName attr);
}
