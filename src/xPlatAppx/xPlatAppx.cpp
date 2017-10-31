#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "ZipObject.hpp"
#include "DirectoryObject.hpp"
#include "ComHelper.hpp"
#include "AppxPackageObject.hpp"
#include "AppxPackaging.hpp"

#include <string>
#include <memory>

#ifdef PLATFORM_APPLE
// on apple platforms, compile with -fvisibility=hidden
#undef XPLATAPPX_API
#define XPLATAPPX_API __attribute__((visibility("default")))

// Initializer.
__attribute__((constructor))
static void initializer(void) {                             // 2
    printf("[%s] initializer()\n", __FILE__);
}

// Finalizer.
__attribute__((destructor))
static void finalizer(void) {                               // 3
    printf("[%s] finalizer()\n", __FILE__);
}

#endif

XPLATAPPX_API HRESULT STDMETHODCALLTYPE UnpackAppx(
    APPX_PACKUNPACK_OPTION packUnpackOptions,
    APPX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination)
{
    return xPlat::ResultOf([&]() {
        // TODO: what if source and destination are something OTHER than a file paths?
        ThrowErrorIfNot(xPlat::Error::InvalidParameter, 
            (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
            "Invalid parameters"
        );

        xPlat::ComPtr<IStream> stream;
        ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream.get()));
        xPlat::AppxPackageObject appx(validationOption, std::make_unique<xPlat::ZipObject>(stream));

        xPlat::DirectoryObject to(utf8Destination);
        appx.Unpack(packUnpackOptions, to);
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE PackAppx(
    APPX_PACKUNPACK_OPTION packUnpackOptions,
    APPX_VALIDATION_OPTION validationOption,
    char* utf8FolderToPack,
    char* utf8CertificatePath,
    char* utf8Destination)
{
    return xPlat::ResultOf([&]() {
        // TODO: what if source and destination are something OTHER than a file paths?
        ThrowErrorIfNot(xPlat::Error::InvalidParameter,
            (utf8FolderToPack != nullptr && utf8Destination != nullptr && utf8CertificatePath != nullptr), 
            "Invalid parameters"
        );

        xPlat::ComPtr<IStream> stream;
        ThrowHrIfFailed(CreateStreamOnFile(utf8Destination, false, &stream.get()));
        xPlat::AppxPackageObject appx(validationOption, std::move(std::make_unique<xPlat::ZipObject>(std::move(stream))));

        xPlat::DirectoryObject from(utf8FolderToPack);
        appx.Pack(packUnpackOptions, utf8CertificatePath, from);
        appx.CommitChanges();
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream)
{
    return xPlat::ResultOf([&]() {
        auto file = std::make_unique<xPlat::FileStream>(utf8File, forRead ? xPlat::FileStream::Mode::READ : xPlat::FileStream::Mode::WRITE_UPDATE);
        UuidOfImpl<IStream> uuid;
        return file->QueryInterface(uuid.iid, stream);
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    return xPlat::ResultOf([&]() {
        auto factory = std::make_unique<AppxFactory>(memalloc, memfree);
        UuidOfImpl<IAppxFactory> uuid;
        return factory->QueryInterface(uuid.iid, appxFactory);
    });
}

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    #ifdef WIN32
        return CoCreateAppxFactory(CoTaskMemAlloc, CoTaskMemFree, validationOption, appxFactory);
    #else
        return CoCreateAppxFactory(new, delete[], validationOption, AppxFactory);
    #endif
}    