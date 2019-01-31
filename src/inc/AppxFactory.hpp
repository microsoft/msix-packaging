//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include "ComHelper.hpp"
#include "MSIXFactory.hpp"
#include "IXml.hpp"
#include "StorageObject.hpp"

#include <string>
#include <vector>

namespace MSIX {
    class AppxFactory final : public ComClass<AppxFactory, IMsixFactory, IAppxFactory, IXmlFactory, IAppxBundleFactory, IMsixFactoryOverrides, IAppxFactoryUtf8>
    {
    public:
        AppxFactory(MSIX_VALIDATION_OPTION validationOptions, MSIX_APPLICABILITY_OPTIONS applicability, COTASKMEMALLOC* memalloc, COTASKMEMFREE* memfree ) : 
            m_validationOptions(validationOptions), m_applicabilityFlags(applicability), m_memalloc(memalloc), m_memfree(memfree)
        {
            ThrowErrorIf(Error::InvalidParameter, (m_memalloc == nullptr || m_memfree == nullptr), "allocator/deallocator pair not specified.")
            ComPtr<IMsixFactory> self;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IMsixFactory>::iid, reinterpret_cast<void**>(&self)));
            m_xmlFactory = CreateXmlFactory(self.Get());
        }

        ~AppxFactory() {}

        // IAppxFactory
        HRESULT STDMETHODCALLTYPE CreatePackageWriter(IStream* outputStream, APPX_PACKAGE_SETTINGS* , IAppxPackageWriter** packageWriter) noexcept override;
        HRESULT STDMETHODCALLTYPE CreatePackageReader (IStream* inputStream, IAppxPackageReader** packageReader) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateManifestReader(IStream* inputStream, IAppxManifestReader** manifestReader) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateBlockMapReader(IStream* inputStream, IAppxBlockMapReader** blockMapReader) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(IStream* blockMapStream, LPCWSTR signatureFileName, IAppxBlockMapReader** blockMapReader) noexcept override;

        // IAppxBundleFactory
        HRESULT STDMETHODCALLTYPE CreateBundleWriter(IStream *outputStream, UINT64 bundleVersion, IAppxBundleWriter **bundleWriter) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateBundleReader(IStream *inputStream, IAppxBundleReader **bundleReader) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateBundleManifestReader(IStream *inputStream, IAppxBundleManifestReader **manifestReader) noexcept override;

        // IMsixFactory
        HRESULT MarshalOutString(std::string& internal, LPWSTR *result) noexcept override;
        HRESULT MarshalOutWstring(std::wstring& internal, LPWSTR* result) noexcept override;
        HRESULT MarshalOutStringUtf8(std::string& internal, LPSTR* result) noexcept override;
        HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) noexcept override;
        MSIX_VALIDATION_OPTION GetValidationOptions() override { return m_validationOptions; }
        ComPtr<IStream> GetResource(const std::string& resource) override;

        // IXmlFactory
        MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const ComPtr<IStream>& stream) override
        {   
            return m_xmlFactory->CreateDomFromStream(footPrintType, stream);
        }

        // IMsixFactoryOverrides
        HRESULT STDMETHODCALLTYPE SpecifyExtension(MSIX_FACTORY_EXTENSION name, IUnknown* extension) noexcept override;
        HRESULT STDMETHODCALLTYPE GetCurrentSpecifiedExtension(MSIX_FACTORY_EXTENSION name, IUnknown** extension) noexcept override;

        // IAppxFactoryUtf8
        HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(IStream* blockMapStream, LPCSTR signatureFileName, IAppxBlockMapReader** blockMapReader) noexcept override;

        ComPtr<IXmlFactory> m_xmlFactory;
        COTASKMEMALLOC* m_memalloc;
        COTASKMEMFREE*  m_memfree;
        MSIX_VALIDATION_OPTION m_validationOptions;
        ComPtr<IStorageObject> m_resourcezip;
        std::vector<std::uint8_t> m_resourcesVector;
        MSIX_APPLICABILITY_OPTIONS m_applicabilityFlags;
        ComPtr<IMsixStreamFactory> m_streamFactory;
        ComPtr<IMsixApplicabilityLanguagesEnumerator> m_applicabilityLanguagesEnumerator;

    private:
        template<typename T>
        void MarshalOutStringHelper(std::size_t size, T* from, T** to);
    };
}