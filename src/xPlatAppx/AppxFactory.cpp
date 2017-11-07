#include "AppxFactory.hpp"
#include "UnicodeConversion.hpp"
#include "Exceptions.hpp"
#include "ZipObject.hpp"
#include "AppxPackageObject.hpp"

namespace xPlat {
    // IAppxFactory
    HRESULT STDMETHODCALLTYPE AppxFactory::CreatePackageWriter (
        IStream* outputStream,
        APPX_PACKAGE_SETTINGS* ,//settings, TODO: plumb this through
        IAppxPackageWriter** packageWriter)
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxFactory::CreatePackageReader (
        IStream* inputStream,
        IAppxPackageReader** packageReader)
    {
        return ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (packageReader == nullptr || *packageReader != nullptr), "Invalid parameter");
            ComPtr<IxPlatFactory> self;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IxPlatFactory>::iid, reinterpret_cast<void**>(&self)));
            ComPtr<IStorageObject> zip(new ZipObject(self.Get(), inputStream));
            ComPtr<IAppxPackageReader> result(new AppxPackageObject(m_validationOptions, zip.Get()));
            *packageReader = result.Detach();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateManifestReader(
        IStream* inputStream,
        IAppxManifestReader** manifestReader)
    {
        return ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateBlockMapReader (
        IStream* inputStream,
        IAppxBlockMapReader** blockMapReader)
    {
        return ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateValidatedBlockMapReader (
        IStream* blockMapStream,
        LPCWSTR signatureFileName,
        IAppxBlockMapReader** blockMapReader)
    {
        return ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT AppxFactory::MarshalOutString(std::string& internal, LPWSTR *result)
    {
        return ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (result == nullptr || *result != nullptr), "bad pointer" );                
            auto intermediate = utf8_to_utf16(internal);
            std::size_t countBytes = sizeof(wchar_t)*(internal.size()+1);
            *result = reinterpret_cast<LPWSTR>(m_memalloc(countBytes));
            ThrowErrorIfNot(Error::OutOfMemory, (*result), "Allocation failed!");
            std::memset(reinterpret_cast<void*>(*result), 0, countBytes);
            std::memcpy(reinterpret_cast<void*>(*result),
                        reinterpret_cast<void*>(const_cast<wchar_t*>(intermediate.c_str())),
                        countBytes - sizeof(wchar_t));
        });
    }

} // namespace xPlat 
