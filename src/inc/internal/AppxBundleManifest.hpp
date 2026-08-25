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

// {ff82ffcd-747a-4df9-8879-853ab9dd15a1}
#ifndef WIN32
interface IBundleInfo : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IBundleInfo : public IUnknown
#endif
{
public:
    virtual std::vector<MSIX::ComPtr<IAppxBundleManifestPackageInfo>>& GetPackages() = 0;
};
MSIX_INTERFACE(IBundleInfo, 0xff82ffcd,0x747a,0x4df9,0x88,0x79,0x85,0x3a,0xb9,0xdd,0x15,0xa1);

// {32e6fcf0-729b-401d-9dbc-f927b494f9af}
#ifndef WIN32
interface IAppxBundleManifestPackageInfoInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxBundleManifestPackageInfoInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetFileName() = 0;
    virtual const std::vector<MSIX::Bcp47Tag>& GetLanguages() = 0;
    virtual const std::vector<UINT32>& GetScales() = 0;
    virtual const std::uint64_t GetOffset() = 0;
    virtual bool HasQualifiedResources() = 0;
};
MSIX_INTERFACE(IAppxBundleManifestPackageInfoInternal, 0x32e6fcf0,0x729b,0x401d,0x9d,0xbc,0xf9,0x27,0xb4,0x94,0xf9,0xaf);

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

    class AppxBundleQualifiedResource final : public MSIX::ComClass<AppxBundleQualifiedResource, IAppxManifestQualifiedResource, IAppxManifestQualifiedResourceUtf8>
    {
    public:
        AppxBundleQualifiedResource(IMsixFactory* factory, const std::string& language) : m_factory(factory), m_language(language) {}

        AppxBundleQualifiedResource(IMsixFactory* factory, const UINT32& scale) : m_factory(factory), m_scale(scale) {}

        // IAppxManifestQualifiedResource
        HRESULT STDMETHODCALLTYPE GetLanguage(LPWSTR *language) noexcept override try
        {
            return m_factory->MarshalOutString(m_language, language);
        } CATCH_RETURN();

        // For now we don't having other resources other than language
        HRESULT STDMETHODCALLTYPE GetScale(UINT32 *scale) noexcept override
        {
            *scale = m_scale;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetDXFeatureLevel(DX_FEATURE_LEVEL *dxFeatureLevel) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        // IAppxManifestQualifiedResourceUtf8
        HRESULT STDMETHODCALLTYPE GetLanguage(LPSTR *language) noexcept override try
        {
            return m_factory->MarshalOutStringUtf8(m_language, language);
        } CATCH_RETURN();

    protected:
        IMsixFactory* m_factory;
        std::string m_language;
        UINT32 m_scale = 0;
    };

    class AppxBundleManifestPackageInfo final : public ComClass<AppxBundleManifestPackageInfo, IAppxBundleManifestPackageInfo, IAppxBundleManifestPackageInfoInternal, IAppxBundleManifestPackageInfoUtf8>
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
            std::vector<UINT32>& scales,
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
        const std::vector<UINT32>& GetScales() override { return m_scales; }
        const std::uint64_t GetOffset() override { return m_offset; }
        bool HasQualifiedResources() override { return !m_languages.empty() || !m_scales.empty(); }

        // IAppxBundleManifestPackageInfoUtf8
        HRESULT STDMETHODCALLTYPE GetFileName(LPSTR *fileName) noexcept override;

    private:
        IMsixFactory* m_factory;
        std::string m_fileName;
        ComPtr<IAppxManifestPackageId> m_packageId;
        std::uint64_t m_size;
        std::uint64_t m_offset;
        std::vector<Bcp47Tag> m_languages;
        std::vector<UINT32> m_scales;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE m_packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
    };
}
