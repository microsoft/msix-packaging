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

        xPlat::AppxPackageObject appx(validationOptions,
            std::make_unique<xPlat::ZipObject>(
                std::make_unique<xPlat::FileStream>(
                    utf8SourcePackage, xPlat::FileStream::Mode::READ
                    )));

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

        xPlat::AppxPackageObject appx(validationOption, std::move(
            std::make_unique<xPlat::ZipObject>(std::move(
                std::make_unique<xPlat::FileStream>(utf8Destination, xPlat::FileStream::Mode::WRITE_UPDATE)
            ))
        ));

        xPlat::DirectoryObject from(utf8FolderToPack);
        appx.Pack(packUnpackOptions, utf8CertificatePath, from);
        appx.CommitChanges();
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{

}

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    
}    