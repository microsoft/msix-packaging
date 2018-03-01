//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include "ComHelper.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include <string>
#include <vector>

// internal interface
EXTERN_C const IID IID_IMSIXFactory;   
#ifndef WIN32
// {1f850db4-32b8-4db6-8bf4-5a897eb611f1}
interface IMSIXFactory : public IUnknown
#else
#include "UnKnwn.h"
#include "Objidl.h"
class IMSIXFactory : public IUnknown
#endif
{
public:
    virtual HRESULT MarshalOutString(std::string& internal, LPWSTR *result) = 0;
    virtual HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) = 0;
    virtual MSIX_VALIDATION_OPTION GetValidationOptions() = 0;
};

SpecializeUuidOfImpl(IMSIXFactory);

namespace MSIX {
    class AppxFactory : public ComClass<AppxFactory, IMSIXFactory, IAppxFactory>
    {
    public:
        AppxFactory(MSIX_VALIDATION_OPTION validationOptions, COTASKMEMALLOC* memalloc, COTASKMEMFREE* memfree ) : 
            m_validationOptions(validationOptions), m_memalloc(memalloc), m_memfree(memfree)
        {
            ThrowErrorIf(Error::InvalidParameter, (m_memalloc == nullptr || m_memfree == nullptr), "allocator/deallocator pair not specified.")
            XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
        }

        ~AppxFactory()
        {
            XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
        }

        // IAppxFactory
        HRESULT STDMETHODCALLTYPE CreatePackageWriter (
            IStream* outputStream,
            APPX_PACKAGE_SETTINGS* ,//settings, TODO: plumb this through
            IAppxPackageWriter** packageWriter) override;           

        HRESULT STDMETHODCALLTYPE CreatePackageReader (IStream* inputStream, IAppxPackageReader** packageReader) override;
        HRESULT STDMETHODCALLTYPE CreateManifestReader(IStream* inputStream, IAppxManifestReader** manifestReader) override ;
        HRESULT STDMETHODCALLTYPE CreateBlockMapReader (IStream* inputStream, IAppxBlockMapReader** blockMapReader) override;

        HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader (
            IStream* blockMapStream,
            LPCWSTR signatureFileName,
            IAppxBlockMapReader** blockMapReader) override;

        // IMSIXFactory
        HRESULT MarshalOutString(std::string& internal, LPWSTR *result) override;
        HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) override;
        MSIX_VALIDATION_OPTION GetValidationOptions() override { return m_validationOptions; }

        COTASKMEMALLOC* m_memalloc;
        COTASKMEMFREE*  m_memfree;
        MSIX_VALIDATION_OPTION m_validationOptions;
    };
}