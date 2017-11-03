#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"
#include "AppxFactory.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace xPlat {
    HRESULT STDMETHODCALLTYPE XplatAppxFactory::CreatePackageWriter(
        IStream* outputStream,
        APPX_PACKAGE_SETTINGS* settings,
        IAppxPackageWriter**packageWriter)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE XplatAppxFactory::CreatePackageReader(
        IStream* inputStream,
        IAppxPackageReader** packageReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE XplatAppxFactory::CreateManifestReader(
        IStream* inputStream,
        IAppxManifestReader** manifestReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }


    HRESULT STDMETHODCALLTYPE XplatAppxFactory::CreateBlockMapReader(
        IStream* inputStream,
        IAppxBlockMapReader** blockMapReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE XplatAppxFactory::CreateValidatedBlockMapReader(
        IStream* blockMapStream,
        LPCWSTR signatureFileName,
        IAppxBlockMapReader** blockMapReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

} // namespace xPlat