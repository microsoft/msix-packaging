//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>

#include "AppxPackaging.hpp"
#include "MSIXFactory.hpp"
#include "ComHelper.hpp"
#include "Applicability.hpp"
#include "VerifierObject.hpp"

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

EXTERN_C const IID IID_IAppxBundleManifestPackageInfoInternal;
#ifndef WIN32
// {32e6fcf0-729b-401d-9dbc-f927b494f9af}
interface IAppxBundleManifestPackageInfoInternal : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IAppxBundleManifestPackageInfoInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetFileName() = 0;
    virtual const std::vector<MSIX::Bcp47Tag>& GetLanguages() = 0;
    virtual const std::uint64_t GetOffset() = 0;
    virtual bool HasQualifiedResources() = 0;
};

SpecializeUuidOfImpl(IAppxBundleManifestPackageInfoInternal);
SpecializeUuidOfImpl(IBundleInfo);

namespace MSIX {
    class AppxBundleManifestObject final : public ComClass<AppxBundleManifestObject, IAppxBundleManifestReader, IVerifierObject, IBundleInfo>
    {
    public:
        AppxBundleManifestObject(IMsixFactory* factory, const ComPtr<IStream>& stream);

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
        IMsixFactory* m_factory;
        ComPtr<IStream> m_stream;
        ComPtr<IAppxManifestPackageId> m_packageId;
        std::vector<ComPtr<IAppxBundleManifestPackageInfo>> m_packages;
    };

    class AppxBundleQualifiedResource final : public MSIX::ComClass<AppxBundleQualifiedResource, IAppxManifestQualifiedResource>
    {
    public:
        AppxBundleQualifiedResource(IMsixFactory* factory, const std::string& language) : m_factory(factory), m_language(language) {}

        // IAppxManifestQualifiedResource
        HRESULT STDMETHODCALLTYPE GetLanguage(LPWSTR *language) noexcept override try
        {
            return m_factory->MarshalOutString(m_language, language);
        } CATCH_RETURN();

        // For now we don't having other resources other than language
        HRESULT STDMETHODCALLTYPE GetScale(UINT32 *scale) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetDXFeatureLevel(DX_FEATURE_LEVEL *dxFeatureLevel) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

    protected:
        IMsixFactory* m_factory;
        std::string m_language;
    };

    class AppxBundleManifestPackageInfo final : public ComClass<AppxBundleManifestPackageInfo, IAppxBundleManifestPackageInfo, IAppxBundleManifestPackageInfoInternal>
    {
    public:
        AppxBundleManifestPackageInfo(
            IMsixFactory* factory,
            const std::string& name,
            const std::string& bundleName,
            const std::string& version,
            const std::uint64_t size,
            const std::uint64_t offset,
            const std::string& resourceId,
            const std::string& architecture,
            const std::string& publisherId,
            std::vector<Bcp47Tag>& languages,
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType);

        // IAppxBundleManifestPackageInfo
        HRESULT STDMETHODCALLTYPE GetPackageType(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE *packageType) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageId(IAppxManifestPackageId **packageId) noexcept override;
        HRESULT STDMETHODCALLTYPE GetFileName(LPWSTR *fileName) noexcept override;
        HRESULT STDMETHODCALLTYPE GetOffset(UINT64 *offset) noexcept override;
        HRESULT STDMETHODCALLTYPE GetSize(UINT64 *size) noexcept override;
        HRESULT STDMETHODCALLTYPE GetResources(IAppxManifestQualifiedResourcesEnumerator **resources) noexcept override;

        // IAppxBundleManifestPackageInfoInternal
        const std::string& GetFileName() override { return m_fileName; }
        const std::vector<Bcp47Tag>& GetLanguages() override { return m_languages; }
        const std::uint64_t GetOffset() override { return m_offset; }
        bool HasQualifiedResources() override { return !m_languages.empty(); }

    private:
        IMsixFactory* m_factory;
        std::string m_fileName;
        ComPtr<IAppxManifestPackageId> m_packageId;
        std::uint64_t m_size;
        std::uint64_t m_offset;
        std::vector<Bcp47Tag> m_languages;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE m_packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
    };
}
