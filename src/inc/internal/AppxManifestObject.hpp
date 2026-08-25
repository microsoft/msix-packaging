//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "AppxPackaging.hpp"
#include "AppxPackageInfo.hpp"
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "VerifierObject.hpp"
#include "IXml.hpp"
#include "UnicodeConversion.hpp"

// {eff6d561-a236-4058-9f1d-8f93633fba4b}
#ifndef WIN32
interface IAppxManifestObject : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxManifestObject : public IUnknown
#endif
{
public:
    virtual const MSIX_PLATFORMS GetPlatform() = 0;
};
MSIX_INTERFACE(IAppxManifestObject, 0xeff6d561,0xa236,0x4058,0x9f,0x1d,0x8f,0x93,0x63,0x3f,0xba,0x4b);

// {daf72e2b-6252-4ed3-a476-5fb656aa0e2c}
#ifndef WIN32
interface IAppxManifestTargetDeviceFamilyInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxManifestTargetDeviceFamilyInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetName() = 0;
};
MSIX_INTERFACE(IAppxManifestTargetDeviceFamilyInternal, 0xdaf72e2b,0x6252,0x4ed3,0xa4,0x76,0x5f,0xb6,0x56,0xaa,0x0e,0x2c);

// {9e2fb304-cec6-4ef0-8df3-10bb2ce714a3}
#ifndef WIN32
interface IAppxManifestQualifiedResourceInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxManifestQualifiedResourceInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetLanguage() = 0;
};
MSIX_INTERFACE(IAppxManifestQualifiedResourceInternal, 0x9e2fb304,0xcec6,0x4ef0,0x8d,0xf3,0x10,0xbb,0x2c,0xe7,0x14,0xa3);

// {6c37be69-b1e0-4764-b892-12a3c5e094a4}
#ifndef WIN32
interface IAppxManifestMainPackageDependencyInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxManifestMainPackageDependencyInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetName() = 0;
    virtual const std::string& GetPublisher() = 0;
    virtual const std::string& GetPackageFamilyName() = 0;
};
MSIX_INTERFACE(IAppxManifestMainPackageDependencyInternal, 0x6c37be69,0xb1e0,0x4764,0xb8,0x92,0x12,0xa3,0xc5,0xe0,0x94,0xa4);

namespace MSIX {

    class AppxManifestTargetDeviceFamily final : public ComClass<AppxManifestTargetDeviceFamily, IAppxManifestTargetDeviceFamily, IAppxManifestTargetDeviceFamilyUtf8, IAppxManifestTargetDeviceFamilyInternal>
    {
    public:
        AppxManifestTargetDeviceFamily(IMsixFactory* factory, std::string& name, std::string& minVersion, std::string& maxVersion) :
            m_factory(factory), m_name(name)
        {
            m_minVersion = DecodeVersionNumber(minVersion);
            m_maxVersion = DecodeVersionNumber(maxVersion);
        }

        // IAppxManifestTargetDeviceFamily
        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* name) noexcept override try
        {
            return m_factory->MarshalOutString(m_name, name);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetMinVersion(UINT64 *minVersion) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (minVersion == nullptr), "bad pointer" );
            *minVersion = m_minVersion;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetMaxVersionTested(UINT64 *maxVersionTested) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (maxVersionTested == nullptr), "bad pointer" );
            *maxVersionTested = m_maxVersion;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        // IAppxManifestTargetDeviceFamilyUtf8
        HRESULT STDMETHODCALLTYPE GetName(LPSTR* name) noexcept override try
        {
            return m_factory->MarshalOutStringUtf8(m_name, name);
        } CATCH_RETURN();

        //IAppxManifestTargetDeviceFamilyInternal
        const std::string& GetName() override
        {
            return m_name;
        }

    protected:
        ComPtr<IMsixFactory> m_factory;
        std::string m_name;
        std::uint64_t m_minVersion;
        std::uint64_t m_maxVersion;
    };

    class AppxManifestApplication final : public ComClass<AppxManifestApplication, IAppxManifestApplication, IAppxManifestApplicationUtf8>
    {
    public:
        AppxManifestApplication(IMsixFactory* factory, std::string& aumid) :
            m_factory(factory), m_aumid(aumid)
        {}

        // IAppxManifestApplication
        HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR name, LPWSTR* value) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetAppUserModelId(LPWSTR* appUserModelId) noexcept override try
        {
            return m_factory->MarshalOutString(m_aumid, appUserModelId);
        } CATCH_RETURN();

        // IAppxManifestApplicationUtf8
        HRESULT STDMETHODCALLTYPE GetStringValue(LPCSTR name, LPSTR* value) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetAppUserModelId(LPSTR* appUserModelId) noexcept override try
        {
            return m_factory->MarshalOutStringUtf8(m_aumid, appUserModelId);
        } CATCH_RETURN();

    protected:
        ComPtr<IMsixFactory> m_factory;
        std::string m_aumid;
    };

    class AppxManifestProperties final : public ComClass<AppxManifestProperties, IAppxManifestProperties, IAppxManifestPropertiesUtf8>
    {
    public:
        AppxManifestProperties(IMsixFactory* factory, std::map<std::string, std::string> stringValues, std::map<std::string, bool> boolValues) :
            m_factory(factory), m_stringValues(stringValues), m_boolValues(boolValues)
        {}

        // IAppxManifestProperties
        HRESULT STDMETHODCALLTYPE GetBoolValue(LPCWSTR name, BOOL* value) noexcept override try
        {
            return GetBoolValue(wstring_to_utf8(name).c_str(), value);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR name, LPWSTR* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr || *value != nullptr), "bad pointer");
            auto result  = m_stringValues.find(wstring_to_utf8(name));
            if (result != m_stringValues.end())
            {
                return m_factory->MarshalOutString(result->second, value);
            }
            return static_cast<HRESULT>(Error::InvalidParameter);
        } CATCH_RETURN();

        // IAppxManifestPropertiesUtf8
        HRESULT STDMETHODCALLTYPE GetBoolValue(LPCSTR name, BOOL* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer");
            auto result  = m_boolValues.find(name);
            if (result != m_boolValues.end())
            {
                *value = result->second ? TRUE : FALSE;
                return static_cast<HRESULT>(Error::OK);
            }
            return static_cast<HRESULT>(Error::InvalidParameter);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetStringValue(LPCSTR name, LPSTR* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr || *value != nullptr), "bad pointer");
            auto result  = m_stringValues.find(name);
            if (result != m_stringValues.end())
            {
                return m_factory->MarshalOutStringUtf8(result->second, value);
            }
            return static_cast<HRESULT>(Error::InvalidParameter);
        } CATCH_RETURN();

        protected:
            ComPtr<IMsixFactory> m_factory;
            std::map<std::string, std::string> m_stringValues;
            std::map<std::string, bool> m_boolValues;
    };

    // TODO: add IAppxManifestPackageDependency2 if needed
    class AppxManifestPackageDependency final : public ComClass<AppxManifestPackageDependency, IAppxManifestPackageDependency, IAppxManifestPackageDependencyUtf8>
    {
    public:
        AppxManifestPackageDependency(IMsixFactory* factory, std::string& minVersion, std::string& name, std::string& publisher) :
            m_factory(factory), m_name(name), m_publisher(publisher)
        {
            m_minVersion = DecodeVersionNumber(minVersion);
        }

        // IAppxManifestPackageDependency
        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* name) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (name == nullptr || *name != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_name, name);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPublisher(LPWSTR* publisher) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (publisher == nullptr || *publisher != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_publisher, publisher);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetMinVersion(UINT64* minVersion) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (minVersion == nullptr), "bad pointer");
            *minVersion = m_minVersion;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        // IAppxManifestPackageDependencyUtf8
        HRESULT STDMETHODCALLTYPE GetName(LPSTR* name) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (name == nullptr || *name != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_name, name);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPublisher(LPSTR* publisher) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (publisher == nullptr || *publisher != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_publisher, publisher);
        } CATCH_RETURN();

    protected:
        ComPtr<IMsixFactory> m_factory;
        UINT64 m_minVersion;
        std::string m_publisher;
        std::string m_name;
    };

    class AppxManifestOptionalPackageInfo final : public ComClass<AppxManifestOptionalPackageInfo, IAppxManifestOptionalPackageInfo, IAppxManifestOptionalPackageInfoUtf8>
    {
    public:
        AppxManifestOptionalPackageInfo(IMsixFactory* factory, const std::string& mainPackageName) :
            m_factory(factory), m_mainPackageName(mainPackageName)
        {}

        // IAppxManifestOptionalPackageInfo
        HRESULT STDMETHODCALLTYPE GetIsOptionalPackage(BOOL* isOptionalPackage) noexcept override try
        {
            *isOptionalPackage = m_mainPackageName.empty() ? FALSE : TRUE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetMainPackageName(LPWSTR* mainPackageName) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (mainPackageName == nullptr || *mainPackageName != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_mainPackageName, mainPackageName);
        } CATCH_RETURN();

        // IAppxManifestOptionalPackageInfoUtf8
        HRESULT STDMETHODCALLTYPE GetMainPackageName(LPSTR* mainPackageName) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (mainPackageName == nullptr || *mainPackageName != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_mainPackageName, mainPackageName);
        } CATCH_RETURN();

    protected:
        ComPtr<IMsixFactory> m_factory;
        std::string m_mainPackageName;
    };

    class AppxManifestMainPackageDependency final : public ComClass<AppxManifestMainPackageDependency, IAppxManifestMainPackageDependency, IAppxManifestMainPackageDependencyUtf8, IAppxManifestMainPackageDependencyInternal>
    {
    public:
        AppxManifestMainPackageDependency(IMsixFactory* factory, const std::string& name, const std::string& publisher, const std::string& packageFamilyName) :
            m_factory(factory), m_name(name), m_publisher(publisher), m_packageFamilyName(packageFamilyName)
        {}

        // IAppxManifestMainPackageDependency
        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* name) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (name == nullptr || *name != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_name, name);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPublisher(LPWSTR* publisher) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (publisher == nullptr || *publisher != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_publisher, publisher);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPackageFamilyName(LPWSTR* packageFamilyName) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (packageFamilyName == nullptr || *packageFamilyName != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_packageFamilyName, packageFamilyName);
        } CATCH_RETURN();

        // IAppxManifestMainPackageDependencyUtf8
        HRESULT STDMETHODCALLTYPE GetName(LPSTR* name) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (name == nullptr || *name != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_name, name);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPublisher(LPSTR* publisher) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (publisher == nullptr || *publisher != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_publisher, publisher);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetPackageFamilyName(LPSTR* packageFamilyName) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (packageFamilyName == nullptr || *packageFamilyName != nullptr), "bad pointer");
            return m_factory->MarshalOutStringUtf8(m_packageFamilyName, packageFamilyName);
        } CATCH_RETURN();

        //IAppxManifestMainPackageDependencyInternal
        const std::string& GetName() override
        {
            return m_name;
        }

        const std::string& GetPublisher() override
        {
            return m_publisher;
        }

        const std::string& GetPackageFamilyName() override
        {
            return m_packageFamilyName;
        }

    protected:
        ComPtr<IMsixFactory> m_factory;
        std::string m_name;
        std::string m_publisher;
        std::string m_packageFamilyName;
    };

    class AppxManifestQualifiedResource final : public ComClass<AppxManifestQualifiedResource, IAppxManifestQualifiedResource, IAppxManifestQualifiedResourceUtf8, IAppxManifestQualifiedResourceInternal>
    {
    public:
        AppxManifestQualifiedResource(IMsixFactory* factory, std::string& language, std::string& scale, std::string& DXFeatureLevel) :
            m_factory(factory), m_language(language)
        {
            //TODO: Process and assign scale and DXFeatureLevel
        }

        // IAppxManifestQualifiedResource
        HRESULT STDMETHODCALLTYPE GetLanguage(LPWSTR *language) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (language == nullptr || *language != nullptr), "bad pointer");
            return m_factory->MarshalOutString(m_language, language);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetScale(UINT32 *scale) noexcept override try
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetDXFeatureLevel(DX_FEATURE_LEVEL *dxFeatureLevel) noexcept override try
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        } CATCH_RETURN();

        // IAppxManifestQualifiedResourceUtf8
        HRESULT STDMETHODCALLTYPE GetLanguage(LPSTR *language) noexcept override try
        {
            return m_factory->MarshalOutStringUtf8(m_language, language);
        } CATCH_RETURN();

        // IAppxManifestQualifiedResourceInternal
        const std::string& GetLanguage() override
        {
            return m_language;
        }

    protected:
        ComPtr<IMsixFactory> m_factory;
        std::string m_language;
        std::uint32_t m_scale;
        DX_FEATURE_LEVEL m_DXFeatureLevel;
    };

    // Object backed by AppxManifest.xml
    class AppxManifestObject final : public ComClass<AppxManifestObject, ChainInterfaces<IAppxManifestReader4, IAppxManifestReader3, IAppxManifestReader2, IAppxManifestReader>,
                                                    IAppxManifestReader5, IVerifierObject, IAppxManifestObject, IMsixDocumentElement>
    {
    public:
        AppxManifestObject(IMsixFactory* factory, const ComPtr<IStream>& stream);

        // IAppxManifestReader
        HRESULT STDMETHODCALLTYPE GetPackageId(IAppxManifestPackageId **packageId) noexcept override;
        HRESULT STDMETHODCALLTYPE GetProperties(IAppxManifestProperties **packageProperties) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageDependencies(IAppxManifestPackageDependenciesEnumerator **dependencies) noexcept override;
        HRESULT STDMETHODCALLTYPE GetCapabilities(APPX_CAPABILITIES *capabilities) noexcept override;
        HRESULT STDMETHODCALLTYPE GetResources(IAppxManifestResourcesEnumerator **resources) noexcept override;
        HRESULT STDMETHODCALLTYPE GetDeviceCapabilities(IAppxManifestDeviceCapabilitiesEnumerator **deviceCapabilities) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPrerequisite(LPCWSTR name, UINT64 *value) noexcept override;
        HRESULT STDMETHODCALLTYPE GetApplications(IAppxManifestApplicationsEnumerator **applications) noexcept override;
        HRESULT STDMETHODCALLTYPE GetStream(IStream **manifestStream) noexcept override;

        // IAppxManifestReader2
        HRESULT STDMETHODCALLTYPE GetQualifiedResources(IAppxManifestQualifiedResourcesEnumerator **resources) noexcept override;
                
        // IAppxManifestReader3
        HRESULT STDMETHODCALLTYPE GetCapabilitiesByCapabilityClass(
            APPX_CAPABILITY_CLASS_TYPE capabilityClass,
            IAppxManifestCapabilitiesEnumerator **capabilities) noexcept override;
        HRESULT STDMETHODCALLTYPE GetTargetDeviceFamilies(IAppxManifestTargetDeviceFamiliesEnumerator **targetDeviceFamilies) noexcept override;

        // IAppxManifestReader4
        HRESULT STDMETHODCALLTYPE GetOptionalPackageInfo(IAppxManifestOptionalPackageInfo **optionalPackageInfo) noexcept override;

        // IAppxManifestReader5
        HRESULT STDMETHODCALLTYPE GetMainPackageDependencies(IAppxManifestMainPackageDependenciesEnumerator **mainPackageDependencies) noexcept override;

        // IVerifierObject
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string& GetPublisher() override { NOTSUPPORTED; }

        // IAppxManifestObject
        const MSIX_PLATFORMS GetPlatform() override { return m_platform; }

        // IMsixDocumentElement
        HRESULT STDMETHODCALLTYPE GetDocumentElement(IMsixElement** documentElement) noexcept override;

    protected:
        std::vector<std::string> GetCapabilities(APPX_CAPABILITY_CLASS_TYPE capabilityClass);

        ComPtr<IMsixFactory> m_factory;
        ComPtr<IStream> m_stream;
        ComPtr<IAppxManifestPackageId> m_packageId;
        MSIX_PLATFORMS m_platform = MSIX_PLATFORM_NONE;
        std::vector<ComPtr<IAppxManifestTargetDeviceFamily>> m_tdf;
        ComPtr<IXmlDom> m_dom;
    };
}
