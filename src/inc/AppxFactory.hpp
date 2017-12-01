#pragma once

#include "AppxPackaging.hpp"
#include "AppxWindows.hpp"
#include "ComHelper.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include <string>
#include <vector>

// internal interface
EXTERN_C const IID IID_IxPlatFactory;   
#ifndef WIN32
MIDL_INTERFACE("1f850db4-32b8-4db6-8bf4-5a897eb611f1")
interface IxPlatFactory : public IUnknown
#else
#include "UnKnwn.h"
#include "Objidl.h"
class IxPlatFactory : public IUnknown
#endif
{
public:
    virtual HRESULT MarshalOutString(std::string& internal, LPWSTR *result) = 0;
    virtual HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) = 0;
    virtual APPX_VALIDATION_OPTION GetValidationOptions() = 0;
};

SpecializeUuidOfImpl(IxPlatFactory);

namespace xPlat {
    class AppxFactory : public ComClass<AppxFactory, IxPlatFactory, IAppxFactory>
    {
    public:
        AppxFactory(APPX_VALIDATION_OPTION validationOptions, COTASKMEMALLOC* memalloc, COTASKMEMFREE* memfree ) : 
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

        // IxPlatFactory
        HRESULT MarshalOutString(std::string& internal, LPWSTR *result) override;
        HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) override;
        APPX_VALIDATION_OPTION GetValidationOptions() override { return m_validationOptions; }

        COTASKMEMALLOC* m_memalloc;
        COTASKMEMFREE*  m_memfree;
        APPX_VALIDATION_OPTION m_validationOptions;
    };
}