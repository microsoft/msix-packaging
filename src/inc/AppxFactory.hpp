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
    class AppxFactory : public ComClass<AppxFactory, IMSIXFactory, IAppxFactory, IXmlFactory>
    {
    public:
        AppxFactory(MSIX_VALIDATION_OPTION validationOptions, COTASKMEMALLOC* memalloc, COTASKMEMFREE* memfree ) : 
            m_validationOptions(validationOptions), m_memalloc(memalloc), m_memfree(memfree)
        {
            ThrowErrorIf(Error::InvalidParameter, (m_memalloc == nullptr || m_memfree == nullptr), "allocator/deallocator pair not specified.")
            ComPtr<IMSIXFactory> self;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IMSIXFactory>::iid, reinterpret_cast<void**>(&self)));
            m_xmlFactory = CreateXmlFactory(self.Get());
        }

        ~AppxFactory() {}

        // IAppxFactory
        HRESULT STDMETHODCALLTYPE CreatePackageWriter (
            IStream* outputStream,
            APPX_PACKAGE_SETTINGS* ,//settings, TODO: plumb this through
            IAppxPackageWriter** packageWriter) override;           

        HRESULT STDMETHODCALLTYPE CreatePackageReader (IStream* inputStream, IAppxPackageReader** packageReader) noexcept override;
        HRESULT STDMETHODCALLTYPE CreateManifestReader(IStream* inputStream, IAppxManifestReader** manifestReader) noexcept override ;
        HRESULT STDMETHODCALLTYPE CreateBlockMapReader (IStream* inputStream, IAppxBlockMapReader** blockMapReader) noexcept override;

        HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader (
            IStream* blockMapStream,
            LPCWSTR signatureFileName,
            IAppxBlockMapReader** blockMapReader) noexcept override;

        // IMSIXFactory
        HRESULT MarshalOutString(std::string& internal, LPWSTR *result) noexcept override;
        HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) noexcept override;
        MSIX_VALIDATION_OPTION GetValidationOptions() override { return m_validationOptions; }
        ComPtr<IStream> GetResource(const std::string& resource) override;

        // IXmlFactory
        MSIX::ComPtr<IXmlDom> CreateDomFromStream(XmlContentType footPrintType, const ComPtr<IStream>& stream) override
        {   
            return m_xmlFactory->CreateDomFromStream(footPrintType, stream);
        }

        ComPtr<IXmlFactory> m_xmlFactory;
        COTASKMEMALLOC* m_memalloc;
        COTASKMEMFREE*  m_memfree;
        MSIX_VALIDATION_OPTION m_validationOptions;
        ComPtr<IStorageObject> m_resourcezip;
        std::vector<std::uint8_t> m_resourcesVector;
    };
}