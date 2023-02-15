//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxBundleManifest.hpp"
#include "AppxPackageInfo.hpp"
#include "ComHelper.hpp"
#include "Enumerators.hpp"
#include "Encoding.hpp"
#include "IXml.hpp"

#include <regex>

namespace MSIX {

    AppxBundleManifestObject::AppxBundleManifestObject(IMsixFactory* factory, const ComPtr<IStream>& stream) : m_factory(factory), m_stream(stream)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(m_factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));
        auto dom = xmlFactory->CreateDomFromStream(XmlContentType::AppxBundleManifestXml, stream);
        XmlVisitor visitorIdentity(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxBundleManifestObject* self = reinterpret_cast<AppxBundleManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId.Get()), "There must be only one Identity element at most in AppxBundleManifest.xml");

            const auto& name      = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& publisher = identityNode->GetAttributeValue(XmlAttributeName::Publisher);
            const auto& version   = identityNode->GetAttributeValue(XmlAttributeName::Version);
            ThrowErrorIf(Error::AppxManifestSemanticError, (publisher.empty()), "Invalid Identity element");
            // Bundles don't have a ResourceId attribute in their manifest, but is always ~ for the package identity.
            self->m_packageId = ComPtr<IAppxManifestPackageId>::Make<AppxManifestPackageId>(self->m_factory, name, version, "~", "neutral", publisher);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Bundle_Identity, visitorIdentity);

        struct _context
        {
            AppxBundleManifestObject* self;
            IXmlDom*                  dom;
            std::vector<std::string>  packageNames;
            size_t                    mainPackages;
        };
        _context context = { this, dom.Get(), {} , 0};

        XmlVisitor visitorPackages(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& packageNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            const auto& name = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_FileName);

            std::ostringstream builder;
            builder << "Duplicate file: '" << name << "' specified in AppxBundleManifest.xml.";
            ThrowErrorIf(Error::AppxManifestSemanticError,
                std::find(std::begin(context->packageNames), std::end(context->packageNames), name) != context->packageNames.end(),
                builder.str().c_str());
            context->packageNames.push_back(name);

            const auto& version        = packageNode->GetAttributeValue(XmlAttributeName::Version);
            const auto& resourceId     = packageNode->GetAttributeValue(XmlAttributeName::ResourceId);
            const auto& architecture   = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Architecture);
            const auto& type           = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Type);
            const auto size            = GetNumber<std::uint64_t>(packageNode, XmlAttributeName::Size, 0);
            const auto offset          = GetNumber<std::uint64_t>(packageNode, XmlAttributeName::Bundle_Package_Offset, 0);

            // Default value is resource
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = (type.empty() || type == "resource") ?
                APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE : APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;

            struct _resourcesContext
            {
                std::vector<Bcp47Tag> languages;
                std::vector<UINT32> scales;
                bool                  hasResources;
            };
            _resourcesContext resourcesContext = { {}, {}, false};
            XmlVisitor visitor(static_cast<void*>(&resourcesContext), [](void* c, const ComPtr<IXmlElement>& resourceNode)->bool
            {
                _resourcesContext* resourcesContext = reinterpret_cast<_resourcesContext*>(c);
                const auto& language = resourceNode->GetAttributeValue(XmlAttributeName::Language);
                if (!language.empty()) { resourcesContext->languages.push_back(Bcp47Tag(language, true)); }

                const auto& scale = resourceNode->GetAttributeValue(XmlAttributeName::Scale);
                if (!scale.empty()) 
                { 
                    UINT32 scaleInt = std::stoi(scale);
                    resourcesContext->scales.push_back(scaleInt); 
                }

                resourcesContext->hasResources = true;
                return true;
            });
            context->dom->ForEachElementIn(packageNode, XmlQueryName::Child_Resources_Resource, visitor);

            ComPtr<IAppxManifestPackageIdInternal> packageIdInternal = context->self->m_packageId.As<IAppxManifestPackageIdInternal>();
            auto package = ComPtr<IAppxBundleManifestPackageInfo>::Make<AppxBundleManifestPackageInfo>(
                context->self->m_factory, name, packageIdInternal->GetName(), version, size, offset, resourceId,
                architecture, packageIdInternal->GetPublisher(), resourcesContext.languages, resourcesContext.scales, packageType);
            context->self->m_packages.push_back(std::move(package));

            if(packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
            {
                context->mainPackages++;
            }
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Bundle_Packages_Package, visitorPackages);
        ThrowErrorIf(Error::XmlError, (context.packageNames.size() == 0), "No packages in AppxBundleManifest.xml");
        ThrowErrorIf(Error::AppxManifestSemanticError, (context.mainPackages == 0), "Bundle contains only resource packages");
    }

    HRESULT STDMETHODCALLTYPE AppxBundleManifestObject::GetPackageId(IAppxManifestPackageId **packageId) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageId == nullptr || *packageId != nullptr), "bad pointer");
        auto package = m_packageId;
        *packageId = package.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestObject::GetPackageInfoItems(IAppxBundleManifestPackageInfoEnumerator **packageInfoItems) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageInfoItems == nullptr || *packageInfoItems != nullptr), "bad pointer.");
        *packageInfoItems = ComPtr<IAppxBundleManifestPackageInfoEnumerator>::
            Make<EnumeratorCom<IAppxBundleManifestPackageInfoEnumerator, IAppxBundleManifestPackageInfo>>(m_packages).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestObject::GetStream(IStream **manifestStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (manifestStream == nullptr || *manifestStream != nullptr), "bad pointer");
        auto stream = GetStream();
        LARGE_INTEGER li{0};
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));
        *manifestStream = stream.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    AppxBundleManifestPackageInfo::AppxBundleManifestPackageInfo(
        IMsixFactory* factory,
        const std::string& name,
        const std::string& bundleName,
        const std::string& version,
        const std::uint64_t size,
        const std::uint64_t offset,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher,
        std::vector<Bcp47Tag>& languages,
        std::vector<UINT32>& scales,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType):
        m_factory(factory), m_fileName(name), m_size(size), m_offset(offset), m_languages(std::move(languages)), m_scales(scales), m_packageType(packageType)
    {
        std::regex e (".+\\.((appx)|(msix))");
        ThrowErrorIf(Error::AppxManifestSemanticError, !std::regex_match(m_fileName, e), "Invalid FileName attribute in AppxBundleManifest.xml");
        m_packageId = ComPtr<IAppxManifestPackageId>::Make<AppxManifestPackageId>(factory, bundleName, version, resourceId, architecture, publisher);
    }

    // IAppxBundleManifestPackageInfo
    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetPackageType(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE *packageType) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageType == nullptr), "bad pointer");
        *packageType = m_packageType;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetPackageId(IAppxManifestPackageId **packageId) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageId == nullptr || *packageId != nullptr), "bad pointer");
        auto package = m_packageId;
        *packageId = package.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetFileName(LPWSTR *fileName) noexcept try
    {
        return m_factory->MarshalOutString(m_fileName, fileName);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetOffset(UINT64 *offset) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (offset == nullptr), "bad pointer");
        *offset = m_offset;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetSize(UINT64 *size) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
        *size = m_size;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetResources(IAppxManifestQualifiedResourcesEnumerator **resources) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (resources == nullptr || *resources != nullptr), "bad pointer.");
        std::vector<ComPtr<IAppxManifestQualifiedResource>> m_resources;
        for(auto& bcp47 : m_languages)
        {
            auto resource = ComPtr<IAppxManifestQualifiedResource>::Make<AppxBundleQualifiedResource>(m_factory, bcp47.GetFullTag());
            m_resources.push_back(std::move(resource));
        }

        for (auto& scale : m_scales)
        {
            auto resource = ComPtr<IAppxManifestQualifiedResource>::Make<AppxBundleQualifiedResource>(m_factory, scale);
            m_resources.push_back(std::move(resource));
        }

        *resources = ComPtr<IAppxManifestQualifiedResourcesEnumerator>::
            Make<EnumeratorCom<IAppxManifestQualifiedResourcesEnumerator, IAppxManifestQualifiedResource>>(m_resources).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxBundleManifestPackageInfoUtf8
    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetFileName(LPSTR* fileName) noexcept try
    {
        return m_factory->MarshalOutStringUtf8(m_fileName, fileName);
    } CATCH_RETURN();

}
