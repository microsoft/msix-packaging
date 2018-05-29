//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxManifestObject.hpp"
#include "UnicodeConversion.hpp"
#include "Encoding.hpp"
#include "Enumerators.hpp"

namespace MSIX {

    struct TargetDeviceFamilyEntry
    {
        const char* tdf;
        const MSIX_PLATFORMS platform;

        TargetDeviceFamilyEntry(const char* t, const MSIX_PLATFORMS p) : tdf(t), platform(p) {}

        inline bool operator==(const char* otherTdf) const {
            return 0 == strcmp(tdf, otherTdf);
        }
    };

    // ALL THE TargetDeviceFamily ENTRIES MUST BE LOWER-CASE
    static const TargetDeviceFamilyEntry targetDeviceFamilyList[] = {
        TargetDeviceFamilyEntry(u8"windows.universal",      MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.mobile",         MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.desktop",        MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.xbox",           MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.team",           MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.holographic",    MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.iot",            MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"apple.ios.all",          MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.phone",        MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.tablet",       MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.tv",           MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.watch",        MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.macos.all",        MSIX_PLATFORM_MACOS),
        TargetDeviceFamilyEntry(u8"google.android.all",     MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.phone",   MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.tablet",  MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.desktop", MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.tv",      MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.watch",   MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"windows7.desktop",       MSIX_PLATFORM_WINDOWS7),
        TargetDeviceFamilyEntry(u8"windows8.desktop",       MSIX_PLATFORM_WINDOWS8),
        TargetDeviceFamilyEntry(u8"linux.all",              MSIX_PLATFORM_LINUX),
        TargetDeviceFamilyEntry(u8"web.edge.all",           MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.blink.all",          MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.chromium.all",       MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.webkit.all",         MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.safari.all",         MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.all",                MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"platform.all",           static_cast<MSIX_PLATFORMS>(MSIX_PLATFORM_ALL)),
    };

    AppxManifestObject::AppxManifestObject(IMSIXFactory* factory, const ComPtr<IStream>& stream) : m_factory(factory), m_stream(stream)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(m_factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));
        auto dom = xmlFactory->CreateDomFromStream(XmlContentType::AppxManifestXml, stream);
        XmlVisitor visitor(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId.Get()), "There must be only one Identity element at most in AppxManifest.xml");

            const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& architecture   = identityNode->GetAttributeValue(XmlAttributeName::Identity_ProcessorArchitecture);
            const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Identity_Publisher);
            const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Version);
            const auto& resourceId     = identityNode->GetAttributeValue(XmlAttributeName::ResourceId);
            ThrowErrorIf(Error::AppxManifestSemanticError, (publisher.empty()), "Invalid Identity element");
            self->m_packageId = ComPtr<IAppxManifestPackageId>::Make<AppxManifestPackageId>(self->m_factory, name, version, resourceId, architecture, publisher);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Package_Identity, visitor);
        // Have to check for this semantically as not all validating parsers can validate this via schema
        ThrowErrorIfNot(Error::AppxManifestSemanticError, m_packageId, "No Identity element in AppxManifest.xml");

        XmlVisitor visitorTDF(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& tdfNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            auto name = tdfNode->GetAttributeValue(XmlAttributeName::Name);
            auto min = tdfNode->GetAttributeValue(XmlAttributeName::Dependencies_Tdf_MinVersion);
            auto max = tdfNode->GetAttributeValue(XmlAttributeName::Dependencies_Tdf_MaxVersionTested);
            auto tdf = ComPtr<IAppxManifestTargetDeviceFamily>::Make<AppxManifestTargetDeviceFamily>(self->m_factory, name, min, max);
            self->m_tdf.push_back(std::move(tdf));
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            const auto& tdfEntry = std::find(std::begin(targetDeviceFamilyList), std::end(targetDeviceFamilyList), name.c_str());
            ThrowErrorIf(Error::AppxManifestSemanticError, (tdfEntry == std::end(targetDeviceFamilyList)), "Unrecognized TargetDeviceFamily");
            self->m_platform = static_cast<MSIX_PLATFORMS>(self->m_platform | (*tdfEntry).platform);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Package_Dependencies_TargetDeviceFamily, visitorTDF);
        ThrowErrorIf(Error::AppxManifestSemanticError, m_platform == MSIX_PLATFORM_NONE , "Couldn't find TargetDeviceFamily element in AppxManifest.xml");

        XmlVisitor visitorApplication(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& applicationNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            auto appId = applicationNode->GetAttributeValue(XmlAttributeName::Package_Applications_Application_Id);
            auto packageIdInternal = self->m_packageId.As<IAppxManifestPackageIdInternal>();
            auto aumid = packageIdInternal->GetPackageFamilyName() + "!" + appId;
            auto application = ComPtr<IAppxManifestApplication>::Make<AppxManifestApplication>(self->m_factory, aumid);
            // TODO: get other attributes from the Application element and store them a map in AppxManifestApplication
            self->m_applications.push_back(std::move(application));
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Package_Applications_Application, visitorApplication);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPackageId(IAppxManifestPackageId **packageId) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageId == nullptr || *packageId != nullptr), "bad pointer");
        auto package = m_packageId;
        *packageId = package.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetProperties(IAppxManifestProperties **packageProperties) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPackageDependencies(IAppxManifestPackageDependenciesEnumerator **dependencies) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetCapabilities(APPX_CAPABILITIES *capabilities) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetResources(IAppxManifestResourcesEnumerator **resources) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetDeviceCapabilities(IAppxManifestDeviceCapabilitiesEnumerator **deviceCapabilities) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPrerequisite(LPCWSTR name, UINT64 *value) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetApplications(IAppxManifestApplicationsEnumerator **applications) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (applications == nullptr || *applications != nullptr), "bad pointer.");
        *applications = ComPtr<IAppxManifestApplicationsEnumerator>::
            Make<EnumeratorCom<IAppxManifestApplicationsEnumerator,IAppxManifestApplication>>(m_applications).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetStream(IStream **manifestStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (manifestStream == nullptr || *manifestStream != nullptr), "bad pointer");
        auto stream = m_stream;
        *manifestStream = stream.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxManifestReader2
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetQualifiedResources(IAppxManifestQualifiedResourcesEnumerator **resources) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    // IAppxManifestReader3
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetCapabilitiesByCapabilityClass(
        APPX_CAPABILITY_CLASS_TYPE capabilityClass,
        IAppxManifestCapabilitiesEnumerator **capabilities) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetTargetDeviceFamilies(IAppxManifestTargetDeviceFamiliesEnumerator **targetDeviceFamilies) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (targetDeviceFamilies == nullptr || *targetDeviceFamilies != nullptr), "bad pointer.");
        *targetDeviceFamilies = ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator>::
            Make<EnumeratorCom<IAppxManifestTargetDeviceFamiliesEnumerator,IAppxManifestTargetDeviceFamily>>(m_tdf).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    AppxBundleManifestObject::AppxBundleManifestObject(IMSIXFactory* factory, const ComPtr<IStream>& stream) : m_factory(factory), m_stream(stream)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(m_factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));
        auto dom = xmlFactory->CreateDomFromStream(XmlContentType::AppxBundleManifestXml, stream);
        XmlVisitor visitorIdentity(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxBundleManifestObject* self = reinterpret_cast<AppxBundleManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId.Get()), "There must be only one Identity element at most in AppxBundleManifest.xml");

            const auto& name      = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& publisher = identityNode->GetAttributeValue(XmlAttributeName::Identity_Publisher);
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

            std::vector<Bcp47Tag> languages;
            XmlVisitor visitor(static_cast<void*>(&languages), [](void* l, const ComPtr<IXmlElement>& resourceNode)->bool
            {
                std::vector<Bcp47Tag>* languages = reinterpret_cast<std::vector<Bcp47Tag>*>(l);
                const auto& language = resourceNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Resources_Resource_Language);
                if (!language.empty()) { languages->push_back(Bcp47Tag(language)); }
                return true;
            });
            context->dom->ForEachElementIn(packageNode, XmlQueryName::Bundle_Packages_Package_Resources_Resource, visitor);

            if ((packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE) && languages.empty())
            {   // For now, we only support languages resource packages
                return true;
            }

            ComPtr<IAppxManifestPackageIdInternal> packageIdInternal = context->self->m_packageId.As<IAppxManifestPackageIdInternal>();
            auto package = ComPtr<IAppxBundleManifestPackageInfo>::Make<AppxBundleManifestPackageInfo>(
                context->self->m_factory, name, packageIdInternal->GetName(), version, size, offset, resourceId,
                architecture, packageIdInternal->GetPublisher(), languages, packageType);
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
}
