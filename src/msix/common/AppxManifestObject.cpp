//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxManifestObject.hpp"
#include "AppxManifestValidation.hpp"
#include "UnicodeConversion.hpp"
#include "Encoding.hpp"
#include "Enumerators.hpp"
#include "AppxPackageInfo.hpp"

#include <array>

namespace MSIX {

    template<typename T>
    struct Entry
    {
        const char* tdf;
        const T value;

        Entry(const char* t, const T p) : tdf(t), value(p) {}

        inline bool operator==(const char* otherTdf) const {
            return 0 == strcmp(tdf, otherTdf);
        }
    };

    // ALL THE TargetDeviceFamily ENTRIES MUST BE LOWER-CASE
    static const Entry<MSIX_PLATFORMS> targetDeviceFamilyList[] = {
        Entry<MSIX_PLATFORMS>(u8"windows.universal",      MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.mobile",         MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.desktop",        MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.xbox",           MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.team",           MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.holographic",    MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.iot",            MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"windows.server",         MSIX_PLATFORM_WINDOWS10),
        Entry<MSIX_PLATFORMS>(u8"apple.ios.all",          MSIX_PLATFORM_IOS),
        Entry<MSIX_PLATFORMS>(u8"apple.ios.phone",        MSIX_PLATFORM_IOS),
        Entry<MSIX_PLATFORMS>(u8"apple.ios.tablet",       MSIX_PLATFORM_IOS),
        Entry<MSIX_PLATFORMS>(u8"apple.ios.tv",           MSIX_PLATFORM_IOS),
        Entry<MSIX_PLATFORMS>(u8"apple.ios.watch",        MSIX_PLATFORM_IOS),
        Entry<MSIX_PLATFORMS>(u8"apple.macos.all",        MSIX_PLATFORM_MACOS),
        Entry<MSIX_PLATFORMS>(u8"google.android.all",     MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"google.android.phone",   MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"google.android.tablet",  MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"google.android.desktop", MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"google.android.tv",      MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"google.android.watch",   MSIX_PLATFORM_AOSP),
        Entry<MSIX_PLATFORMS>(u8"msixcore.desktop",       MSIX_PLATFORM_CORE),
        Entry<MSIX_PLATFORMS>(u8"msixcore.server",        MSIX_PLATFORM_CORE),
        Entry<MSIX_PLATFORMS>(u8"linux.all",              MSIX_PLATFORM_LINUX),
        Entry<MSIX_PLATFORMS>(u8"web.edge.all",           MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"web.blink.all",          MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"web.chromium.all",       MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"web.webkit.all",         MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"web.safari.all",         MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"web.all",                MSIX_PLATFORM_WEB),
        Entry<MSIX_PLATFORMS>(u8"platform.all",           static_cast<MSIX_PLATFORMS>(MSIX_PLATFORM_ALL)),
    };

    static const Entry<APPX_CAPABILITIES> capabilitiesList[] = {
        Entry<APPX_CAPABILITIES>(u8"internetClient",             APPX_CAPABILITY_INTERNET_CLIENT),
        Entry<APPX_CAPABILITIES>(u8"internetClientServer",       APPX_CAPABILITY_INTERNET_CLIENT_SERVER),
        Entry<APPX_CAPABILITIES>(u8"privateNetworkClientServer", APPX_CAPABILITY_PRIVATE_NETWORK_CLIENT_SERVER),
        Entry<APPX_CAPABILITIES>(u8"documentsLibrary",           APPX_CAPABILITY_DOCUMENTS_LIBRARY),
        Entry<APPX_CAPABILITIES>(u8"picturesLibrary",            APPX_CAPABILITY_PICTURES_LIBRARY),
        Entry<APPX_CAPABILITIES>(u8"videosLibrary",              APPX_CAPABILITY_VIDEOS_LIBRARY),
        Entry<APPX_CAPABILITIES>(u8"musicLibrary",               APPX_CAPABILITY_MUSIC_LIBRARY),
        Entry<APPX_CAPABILITIES>(u8"enterpriseAuthentication",   APPX_CAPABILITY_ENTERPRISE_AUTHENTICATION),
        Entry<APPX_CAPABILITIES>(u8"sharedUserCertificates",     APPX_CAPABILITY_SHARED_USER_CERTIFICATES),
        Entry<APPX_CAPABILITIES>(u8"removableStorage",           APPX_CAPABILITY_REMOVABLE_STORAGE),
        Entry<APPX_CAPABILITIES>(u8"appointments",               APPX_CAPABILITY_APPOINTMENTS),
        Entry<APPX_CAPABILITIES>(u8"contacts",                   APPX_CAPABILITY_CONTACTS),
    };

    AppxManifestObject::AppxManifestObject(IMsixFactory* factory, const ComPtr<IStream>& stream) : m_factory(factory), m_stream(stream)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(m_factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));
        m_dom = xmlFactory->CreateDomFromStream(XmlContentType::AppxManifestXml, stream);

#if VALIDATING
        AppxManifestValidation::ValidateManifest(m_dom.Get());
#endif

        // Parse Identity element
        XmlVisitor visitor(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId.Get()), "There must be only one Identity element at most in AppxManifest.xml");

            const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& architecture   = identityNode->GetAttributeValue(XmlAttributeName::Identity_ProcessorArchitecture);
            const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Publisher);
            const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Version);
            const auto& resourceId     = identityNode->GetAttributeValue(XmlAttributeName::ResourceId);
            ThrowErrorIf(Error::AppxManifestSemanticError, (publisher.empty()), "Invalid Identity element");
            self->m_packageId = ComPtr<IAppxManifestPackageId>::Make<AppxManifestPackageId>(self->m_factory.Get(), name, version, resourceId, architecture, publisher);
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Identity, visitor);
        // Have to check for this semantically as not all validating parsers can validate this via schema
        ThrowErrorIfNot(Error::AppxManifestSemanticError, m_packageId, "No Identity element in AppxManifest.xml");

        // Parse TargetDeviceFamily elements
        XmlVisitor visitorTDF(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& tdfNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            auto name = tdfNode->GetAttributeValue(XmlAttributeName::Name);
            auto min = tdfNode->GetAttributeValue(XmlAttributeName::MinVersion);
            auto max = tdfNode->GetAttributeValue(XmlAttributeName::Dependencies_Tdf_MaxVersionTested);
            auto tdf = ComPtr<IAppxManifestTargetDeviceFamily>::Make<AppxManifestTargetDeviceFamily>(self->m_factory.Get(), name, min, max);
            self->m_tdf.push_back(std::move(tdf));
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            const auto& tdfEntry = std::find(std::begin(targetDeviceFamilyList), std::end(targetDeviceFamilyList), name.c_str());
            // TODO: Here and below; are unknown device families really an error?  I don't think so.
            ThrowErrorIf(Error::AppxManifestSemanticError, (tdfEntry == std::end(targetDeviceFamilyList)), "Unrecognized TargetDeviceFamily");
            self->m_platform = static_cast<MSIX_PLATFORMS>(self->m_platform | (*tdfEntry).value);
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Dependencies_TargetDeviceFamily, visitorTDF);
        ThrowErrorIf(Error::AppxManifestSemanticError, m_platform == MSIX_PLATFORM_NONE , "Couldn't find TargetDeviceFamily element in AppxManifest.xml");
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPackageId(IAppxManifestPackageId **packageId) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageId == nullptr || *packageId != nullptr), "bad pointer");
        auto package = m_packageId;
        *packageId = package.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetProperties(IAppxManifestProperties **packageProperties) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageProperties == nullptr || *packageProperties != nullptr), "bad pointer");

        // Parse elements in Properties element
        std::map<std::string, std::string> stringValues;
        std::map<std::string, bool> boolValues;
        struct _context
        {
            AppxManifestObject* self;
            std::map<std::string, std::string>* stringValues;
            std::map<std::string, bool>* boolValues;

        };
        _context context = { this, &stringValues, &boolValues};
        XmlVisitor visitorProperties(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& propertiesNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);

            // Read first the elements with string values
            struct _contextPropertiesString
            {
                std::map<std::string, std::string>* stringValues;
                std::string value;
                bool wasFound;
            };
            _contextPropertiesString contextPropertiesString = { context->stringValues, "Description", false};
            XmlVisitor visitorString(static_cast<void*>(&contextPropertiesString), [](void* c, const ComPtr<IXmlElement>& node)->bool
            {
                _contextPropertiesString* contextProperties = reinterpret_cast<_contextPropertiesString*>(c);
                auto value = node->GetText();
                contextProperties->stringValues->insert(std::pair<std::string, std::string>(contextProperties->value, value));
                return true;
            });
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_Description, visitorString);
            if (!contextPropertiesString.wasFound)
            {   // If there's no node we need to add an empty string
                context->stringValues->insert(std::pair<std::string, std::string>(contextPropertiesString.value, ""));
            }
            contextPropertiesString.value = "DisplayName";
            contextPropertiesString.wasFound = false;
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_DisplayName, visitorString);
            if (!contextPropertiesString.wasFound)
            {   // If there's no node we need to add an empty string
                context->stringValues->insert(std::pair<std::string, std::string>(contextPropertiesString.value, ""));
            }
            contextPropertiesString.value = "PublisherDisplayName";
            contextPropertiesString.wasFound = false;
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_PublisherDisplayName, visitorString);
            if (!contextPropertiesString.wasFound)
            {   // If there's no node we need to add an empty string
                context->stringValues->insert(std::pair<std::string, std::string>(contextPropertiesString.value, ""));
            }
            contextPropertiesString.value = "Logo";
            contextPropertiesString.wasFound = false;
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_Logo, visitorString);
            if (!contextPropertiesString.wasFound)
            {   // If there's no node we need to add an empty string
                context->stringValues->insert(std::pair<std::string, std::string>(contextPropertiesString.value, ""));
            }

            // Now read the elements win bool values
            struct _contextPropertiesBool
            {
                std::map<std::string, bool>* boolValues;
                std::string value;
                bool wasFound;
            };
            _contextPropertiesBool contextPropertiesBool = { context->boolValues, "Framework", false};
            XmlVisitor visitorBool(static_cast<void*>(&contextPropertiesBool), [](void* c, const ComPtr<IXmlElement>& node)->bool
            {
                _contextPropertiesBool* contextProperties = reinterpret_cast<_contextPropertiesBool*>(c);
                auto value = node->GetText();
                // We expect the manifest to be correct
                contextProperties->boolValues->insert(std::pair<std::string, bool>(contextProperties->value, value == "true"));
                contextProperties->wasFound = true;
                return true;
            });
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_Framework, visitorBool);
            if (!contextPropertiesBool.wasFound)
            {   // False is default value for Framework
                context->boolValues->insert(std::pair<std::string, bool>(contextPropertiesBool.value, false));
            }
            contextPropertiesBool.value = "ResourcePackage";
            contextPropertiesBool.wasFound = false;
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_ResourcePackage, visitorBool);
            if (!contextPropertiesBool.wasFound)
            {   // False is default value for ResourcePackage
                context->boolValues->insert(std::pair<std::string, bool>(contextPropertiesBool.value, false));
            }

            contextPropertiesBool.value = "AllowExecution";
            contextPropertiesBool.wasFound = false;
            context->self->m_dom->ForEachElementIn(propertiesNode, XmlQueryName::Child_AllowExecution, visitorBool);
            if (!contextPropertiesBool.wasFound)
            {   // True is default value for AllowExecution
                context->boolValues->insert(std::pair<std::string, bool>(contextPropertiesBool.value, true));
            }
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Properties, visitorProperties);
        *packageProperties = ComPtr<IAppxManifestProperties>::Make<AppxManifestProperties>(
            m_factory.Get(), std::move(stringValues), std::move(boolValues)).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPackageDependencies(IAppxManifestPackageDependenciesEnumerator **dependencies) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (dependencies == nullptr || *dependencies != nullptr), "bad pointer");
        std::vector<ComPtr<IAppxManifestPackageDependency>> packageDependencies;
        struct _context
        {
            AppxManifestObject* self;
            std::vector<ComPtr<IAppxManifestPackageDependency>>* packageDependencies;
        };
        _context context = { this, &packageDependencies};

        // Parse PackageDependency elements
        XmlVisitor visitorDependencies(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& dependencyNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            auto min = dependencyNode->GetAttributeValue(XmlAttributeName::MinVersion);
            auto name = dependencyNode->GetAttributeValue(XmlAttributeName::Name);
            auto publisher = dependencyNode->GetAttributeValue(XmlAttributeName::Publisher);
            // TODO: get MaxMajorVersionTested if needed
            auto dependency = ComPtr<IAppxManifestPackageDependency>::Make<AppxManifestPackageDependency>(context->self->m_factory.Get(), min, name, publisher);
            context->packageDependencies->push_back(std::move(dependency));
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Dependencies_PackageDependency, visitorDependencies);
        *dependencies = ComPtr<IAppxManifestPackageDependenciesEnumerator>::
            Make<EnumeratorCom<IAppxManifestPackageDependenciesEnumerator,IAppxManifestPackageDependency>>(packageDependencies).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetCapabilities(APPX_CAPABILITIES *capabilities) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (capabilities == nullptr), "bad pointer");

        APPX_CAPABILITIES appxCapabilities = static_cast<APPX_CAPABILITIES>(0);
        auto capabilitiesNames = GetCapabilities(APPX_CAPABILITY_CLASS_GENERAL);
        for (const auto& capability : capabilitiesNames)
        {
            const auto& capabilityEntry = std::find(std::begin(capabilitiesList), std::end(capabilitiesList), capability.c_str());
            // Don't fail if not found as it can be custom capability or from a different namespace.
            if (capabilityEntry != std::end(capabilitiesList))
            {
                appxCapabilities = static_cast<APPX_CAPABILITIES>((appxCapabilities) | (*capabilityEntry).value);
            }
        }
        *capabilities = appxCapabilities;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetResources(IAppxManifestResourcesEnumerator **resources) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (resources == nullptr || *resources != nullptr), "bad pointer");
        // Parse Resource elements.
        std::vector<std::string> appxResources;
        XmlVisitor visitorResource(static_cast<void*>(&appxResources), [](void* r, const ComPtr<IXmlElement>& resourceNode)->bool
        {
            std::vector<std::string>* resources = reinterpret_cast<std::vector<std::string>*>(r);
            auto name = resourceNode->GetAttributeValue(XmlAttributeName::Language);
            resources->push_back(std::move(name));
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Resources_Resource, visitorResource);
        *resources = ComPtr<IAppxManifestResourcesEnumerator>::Make<EnumeratorString<IAppxManifestResourcesEnumerator, IAppxManifestResourcesEnumeratorUtf8>>(m_factory.Get(), appxResources).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetDeviceCapabilities(IAppxManifestDeviceCapabilitiesEnumerator **deviceCapabilities) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    // This method became deprecated as of Windows 8.1, use GetTargetDeviceFamilies instead.
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetPrerequisite(LPCWSTR name, UINT64 *value) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetApplications(IAppxManifestApplicationsEnumerator **applications) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (applications == nullptr || *applications != nullptr), "bad pointer");

        std::vector<ComPtr<IAppxManifestApplication>> apps;
        struct _context
        {
            AppxManifestObject* self;
            std::vector<ComPtr<IAppxManifestApplication>>* apps;
        };
        _context context = { this, &apps};

        // Parse Application elements
        XmlVisitor visitorApplication(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& applicationNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            auto appId = applicationNode->GetAttributeValue(XmlAttributeName::Package_Applications_Application_Id);
            auto packageIdInternal = context->self->m_packageId.As<IAppxManifestPackageIdInternal>();
            auto aumid = packageIdInternal->GetPackageFamilyName() + "!" + appId;
            auto application = ComPtr<IAppxManifestApplication>::Make<AppxManifestApplication>(context->self->m_factory.Get(), aumid);
            // TODO: get other attributes from the Application element and store them a map in AppxManifestApplication
            // or make the AppxManifestApplication have a IXmlElement member to get attributes at will.
            context->apps->push_back(std::move(application));
            return true;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Applications_Application, visitorApplication);
        *applications = ComPtr<IAppxManifestApplicationsEnumerator>::
            Make<EnumeratorCom<IAppxManifestApplicationsEnumerator,IAppxManifestApplication>>(apps).Detach();
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
        ThrowErrorIf(Error::InvalidParameter, (capabilities == nullptr), "bad pointer");

        *capabilities = nullptr;
        auto capabilitiesNames = GetCapabilities(capabilityClass);
        *capabilities = ComPtr<IAppxManifestCapabilitiesEnumerator>::Make<EnumeratorString<IAppxManifestCapabilitiesEnumerator, IAppxManifestCapabilitiesEnumeratorUtf8>>(m_factory.Get(), capabilitiesNames).Detach();
        return static_cast<HRESULT>(Error::OK);
    }

    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetTargetDeviceFamilies(IAppxManifestTargetDeviceFamiliesEnumerator **targetDeviceFamilies) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (targetDeviceFamilies == nullptr || *targetDeviceFamilies != nullptr), "bad pointer");
        *targetDeviceFamilies = ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator>::
            Make<EnumeratorCom<IAppxManifestTargetDeviceFamiliesEnumerator, IAppxManifestTargetDeviceFamily>>(m_tdf).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxManifestReader4
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetOptionalPackageInfo(IAppxManifestOptionalPackageInfo **optionalPackageInfo) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (optionalPackageInfo == nullptr || *optionalPackageInfo != nullptr), "bad pointer.");

        struct _context
        {
            std::string mainPackageName;
        };
        _context context = {};

        // Parse MainPackageDependency elements
        XmlVisitor visitorMainPackageDependencies(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& dependencyNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            auto name = dependencyNode->GetAttributeValue(XmlAttributeName::Name);
            context->mainPackageName = name;
            return false;
        });
        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Dependencies_MainPackageDependency, visitorMainPackageDependencies);
        *optionalPackageInfo = ComPtr<IAppxManifestOptionalPackageInfo>::Make<AppxManifestOptionalPackageInfo>(this->m_factory.Get(), context.mainPackageName).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxManifestReader5
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetMainPackageDependencies(IAppxManifestMainPackageDependenciesEnumerator **mainPackageDependencies) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (mainPackageDependencies == nullptr || *mainPackageDependencies != nullptr), "bad pointer.");
        std::vector<ComPtr<IAppxManifestMainPackageDependency>> packageDependencies;

        struct _context
        {
            AppxManifestObject* self;
            std::vector<ComPtr<IAppxManifestMainPackageDependency>>* packageDependencies;
        };
        _context context = { this, &packageDependencies };

        // Parse MainPackageDependency elements
        XmlVisitor visitorMainPackageDependencies(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& dependencyNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            auto name = dependencyNode->GetAttributeValue(XmlAttributeName::Name);
            auto publisher = dependencyNode->GetAttributeValue(XmlAttributeName::Publisher);
            std::string packageFamilyName;

            // if no publisher for the main package dependency is specified, we default to the publisher of the optional package itself
            if (publisher.empty())
            {
                auto packageIdInternal = context->self->m_packageId.As<IAppxManifestPackageIdInternal>();
                packageFamilyName = packageIdInternal->GetPackageFamilyName();
            }
            else
            {
                // Convert publisher to publisher hash so we can build package family name
                std::string publisherHash = ComputePublisherId(publisher);
                packageFamilyName = name + "_" + publisherHash;
            }

            auto dependency = ComPtr<IAppxManifestMainPackageDependency>::Make<AppxManifestMainPackageDependency>(context->self->m_factory.Get(), name, publisher, packageFamilyName);
            context->packageDependencies->push_back(std::move(dependency));
            return true;
        });

        m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Dependencies_MainPackageDependency, visitorMainPackageDependencies);
        *mainPackageDependencies = ComPtr<IAppxManifestMainPackageDependenciesEnumerator>::
            Make<EnumeratorCom<IAppxManifestMainPackageDependenciesEnumerator, IAppxManifestMainPackageDependency>>(packageDependencies).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IMsixDocumentElement
    HRESULT STDMETHODCALLTYPE AppxManifestObject::GetDocumentElement(IMsixElement** documentElement) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (documentElement == nullptr || *documentElement != nullptr), "bad pointer");
        *documentElement = m_dom->GetDocument().As<IMsixElement>().Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // Helper to get capabilities from the manifest
    std::vector<std::string> AppxManifestObject::GetCapabilities(APPX_CAPABILITY_CLASS_TYPE capabilityClass)
    {
        ThrowErrorIf(Error::InvalidParameter, capabilityClass > APPX_CAPABILITY_CLASS_CUSTOM || capabilityClass < APPX_CAPABILITY_CLASS_DEFAULT, 
            "Invalid capability class.");

        std::vector<std::string> capabilitiesNames;
        struct _context
        {
            APPX_CAPABILITY_CLASS_TYPE capabilityClass;
            std::vector<std::string>& capabilitiesNames;
        };
        _context context = { capabilityClass, capabilitiesNames};

        // Parse Capability elements.
        if (capabilityClass != APPX_CAPABILITY_CLASS_CUSTOM)
        {
            XmlVisitor visitorCapabilities(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& capabilitiesNode)->bool
            {
                _context* context = reinterpret_cast<_context*>(c);
                std::string prefix = capabilitiesNode->GetPrefix();
                auto name = capabilitiesNode->GetAttributeValue(XmlAttributeName::Name);

                static std::array<std::string, 11> generalCapabilities = 
                {
                    "foundation",
                    "uap",
                    "win10foundation",
                    "win10uap",
                    "uap2",
                    "uap3",
                    "uap4",
                    "uap6",
                    "uap7",
                    "win10mobile",
                    "", // If no prefix then default namespace for AppxManifest is win10foundation.
                };

                static std::array<std::string, 2> restrictedCapabilities = 
                {
                    "rescap",
                    "win10rescap",
                };

                static std::array<std::string, 2> windowsCapabilities = 
                {
                    "wincap",
                    "win10wincap",
                };

                if (context->capabilityClass == APPX_CAPABILITY_CLASS_GENERAL ||
                    context->capabilityClass == APPX_CAPABILITY_CLASS_DEFAULT ||
                    context->capabilityClass == APPX_CAPABILITY_CLASS_ALL)
                {
                    auto found = std::find(generalCapabilities.begin(), generalCapabilities.end(), prefix);
                    if (found != generalCapabilities.end())
                    {
                        context->capabilitiesNames.push_back(name);
                        return true;
                    }
                }

                if (context->capabilityClass == APPX_CAPABILITY_CLASS_RESTRICTED ||
                    context->capabilityClass == APPX_CAPABILITY_CLASS_ALL)
                { 
                    auto found = std::find(restrictedCapabilities.begin(), restrictedCapabilities.end(), prefix);
                    if (found != restrictedCapabilities.end())
                    {
                        context->capabilitiesNames.push_back(name);
                        return true;
                    }
                }

                if (context->capabilityClass == APPX_CAPABILITY_CLASS_WINDOWS ||
                    context->capabilityClass == APPX_CAPABILITY_CLASS_ALL)
                {
                    auto found = std::find(windowsCapabilities.begin(), windowsCapabilities.end(), prefix);
                    if (found != windowsCapabilities.end())
                    {
                        context->capabilitiesNames.push_back(name);
                        return true;
                    }
                }

                return true;
            });
            m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Capabilities_Capability, visitorCapabilities);
        }

        if (capabilityClass == APPX_CAPABILITY_CLASS_CUSTOM || capabilityClass == APPX_CAPABILITY_CLASS_ALL)
        {
            XmlVisitor visitorCustomCapabilities(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& capabilitiesNode)->bool
            {
                _context* context = reinterpret_cast<_context*>(c);
                auto name = capabilitiesNode->GetAttributeValue(XmlAttributeName::Name);
                context->capabilitiesNames.push_back(name);
                return true;
            });
            m_dom->ForEachElementIn(m_dom->GetDocument(), XmlQueryName::Package_Capabilities_CustomCapability, visitorCustomCapabilities);
        }

        return capabilitiesNames;
    }
}
