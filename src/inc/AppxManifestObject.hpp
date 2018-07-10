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

EXTERN_C const IID IID_IAppxManifestObject;
#ifndef WIN32
// {eff6d561-a236-4058-9f1d-8f93633fba4b}
interface IAppxManifestObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IAppxManifestObject : public IUnknown
#endif
{
public:
    virtual const MSIX_PLATFORMS GetPlatform() = 0;
};

EXTERN_C const IID IID_IBundleInfo;
#ifndef WIN32
// {ff82ffcd-747a-4df9-8879-853ab9dd15a1}
interface IBundleInfo : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IBundleInfo : public IUnknown
#endif
{
public:
    virtual std::vector<MSIX::ComPtr<IAppxBundleManifestPackageInfo>>& GetPackages() = 0;
};

SpecializeUuidOfImpl(IAppxManifestObject);
SpecializeUuidOfImpl(IBundleInfo);

namespace MSIX {

    class AppxManifestTargetDeviceFamily final : public ComClass<AppxManifestTargetDeviceFamily, IAppxManifestTargetDeviceFamily>
    {
    public:
        AppxManifestTargetDeviceFamily(IMSIXFactory* factory, std::string& name, std::string& minVersion, std::string& maxVersion) :
            m_factory(factory), m_name(name)
        {
            m_minVersion = DecodeVersionNumber(minVersion);
            m_maxVersion = DecodeVersionNumber(maxVersion);
        }

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

    protected:
        IMSIXFactory* m_factory;
        std::string m_name;
        std::uint64_t m_minVersion;
        std::uint64_t m_maxVersion;
    };

    class AppxManifestApplication final : public ComClass<AppxManifestApplication, IAppxManifestApplication>
    {
    public:
        AppxManifestApplication(IMSIXFactory* factory, std::string& aumid) :
            m_factory(factory), m_aumid(aumid)
        {}

        HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR name, LPWSTR* value) override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetAppUserModelId(LPWSTR* appUserModelId) noexcept override try
        {
            return m_factory->MarshalOutString(m_aumid, appUserModelId);
        } CATCH_RETURN();

    protected:
        IMSIXFactory* m_factory;
        std::string m_aumid;
    };

    class AppxManifestProperties final : public ComClass<AppxManifestProperties, IAppxManifestProperties>
    {
    public:
        AppxManifestProperties(IMSIXFactory* factory, std::map<std::string, std::string> stringValues, std::map<std::string, bool> boolValues) :
            m_factory(factory), m_stringValues(stringValues), m_boolValues(boolValues)
        {}

        HRESULT STDMETHODCALLTYPE GetBoolValue(LPCWSTR name, BOOL* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr), "bad pointer");
            auto result  = m_boolValues.find(utf16_to_utf8(name));
            if (result != m_boolValues.end())
            {
                *value = result->second ? TRUE : FALSE;
                return static_cast<HRESULT>(Error::OK);
            }
            return static_cast<HRESULT>(Error::InvalidParameter);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR name, LPWSTR* value) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (value == nullptr || *value != nullptr), "bad pointer");
            auto result  = m_stringValues.find(utf16_to_utf8(name));
            if (result != m_stringValues.end())
            {
                return m_factory->MarshalOutString(result->second, value);
            }
            return static_cast<HRESULT>(Error::InvalidParameter);
        } CATCH_RETURN();

        protected:
            IMSIXFactory* m_factory;
            std::map<std::string, std::string> m_stringValues;
            std::map<std::string, bool> m_boolValues;
    };

    // TODO: add IAppxManifestPackageDependency2 if needed
    class AppxManifestPackageDependency final : public ComClass<AppxManifestPackageDependency, IAppxManifestPackageDependency>
    {
    public:
        AppxManifestPackageDependency(IMSIXFactory* factory, std::string& minVersion, std::string& name, std::string& publisher) :
            m_factory(factory), m_name(name), m_publisher(publisher)
        {
            m_minVersion = DecodeVersionNumber(minVersion);
        }

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

    protected:
        IMSIXFactory* m_factory;
        UINT64 m_minVersion;
        std::string m_publisher;
        std::string m_name;
    };

    // Object backed by AppxManifest.xml
    class AppxManifestObject final : public ComClass<AppxManifestObject, ChainInterfaces<IAppxManifestReader3, IAppxManifestReader2, IAppxManifestReader>,
                                                     IVerifierObject, IAppxManifestObject, IMSIXDocumentElement>
    {
    public:
        AppxManifestObject(IMSIXFactory* factory, const ComPtr<IStream>& stream);

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

        // IVerifierObject
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string& GetPublisher() override { NOTSUPPORTED; }

        // IAppxManifestObject
        const MSIX_PLATFORMS GetPlatform() override { return m_platform; }

        // IMSIXDocumentElement
        HRESULT STDMETHODCALLTYPE GetDocumentElement(IMSIXElement** documentElement) noexcept override;

    protected:
        IMSIXFactory* m_factory;
        ComPtr<IStream> m_stream;
        ComPtr<IAppxManifestPackageId> m_packageId;
        MSIX_PLATFORMS m_platform = MSIX_PLATFORM_NONE;
        std::vector<ComPtr<IAppxManifestTargetDeviceFamily>> m_tdf;
        ComPtr<IXmlDom> m_dom;
    };

    class AppxBundleManifestObject final : public ComClass<AppxBundleManifestObject, IAppxBundleManifestReader, IVerifierObject, IBundleInfo>
    {
    public:
        AppxBundleManifestObject(IMSIXFactory* factory, const ComPtr<IStream>& stream);

         // IVerifierObject
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string& GetPublisher() override { NOTSUPPORTED; }

        // IAppxBundleManifestReader
        HRESULT STDMETHODCALLTYPE GetPackageId(IAppxManifestPackageId **packageId) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageInfoItems(IAppxBundleManifestPackageInfoEnumerator **packageInfoItems) noexcept override;
        HRESULT STDMETHODCALLTYPE GetStream(IStream **manifestStream) noexcept override;

        // IBundleInfo
        std::vector<ComPtr<IAppxBundleManifestPackageInfo>>& GetPackages() override { return m_packages; }

    protected:
        IMSIXFactory* m_factory;
        ComPtr<IStream> m_stream;
        ComPtr<IAppxManifestPackageId> m_packageId;
        std::vector<ComPtr<IAppxBundleManifestPackageInfo>> m_packages;
    };
}
