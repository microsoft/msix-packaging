#include "ComHelper.hpp"
#include "AppxPackaging.hpp"

namespace xPlat {
    class XplatAppxFactory : public xPlat::ComClass<XplatAppxFactory, IAppxFactory>
    {
    public:
        AppxFactory() { }

        // IAppxFactory
        HRESULT STDMETHODCALLTYPE CreatePackageWriter(
            IStream* outputStream,
            APPX_PACKAGE_SETTINGS* settings,
            IAppxPackageWriter**packageWriter)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE CreatePackageReader(
            IStream* inputStream,
            IAppxPackageReader** packageReader)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE CreateManifestReader(
            IStream* inputStream,
            IAppxManifestReader** manifestReader)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }


        HRESULT STDMETHODCALLTYPE CreateBlockMapReader(
            IStream* inputStream,
            IAppxBlockMapReader** blockMapReader)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(
            IStream* blockMapStream,
            LPCWSTR signatureFileName,
            IAppxBlockMapReader** blockMapReader)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }
    };
}