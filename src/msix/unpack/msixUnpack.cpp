//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "DirectoryObject.hpp"
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxPackageObject.hpp"
#include "AppxFactory.hpp"

#include <string>
#include <memory>
#include <cstdlib>
#include <functional>

#ifndef WIN32
// on non-win32 platforms, compile with -fvisibility=hidden
#undef MSIX_API
#define MSIX_API __attribute__((visibility("default")))

// Initializer.
__attribute__((constructor))
static void initializer(void) {
//    printf("[%s] initializer()\n", __FILE__);
}

// Finalizer.
__attribute__((destructor))
static void finalizer(void) {
//    printf("[%s] finalizer()\n", __FILE__);
}

#endif

extern LPVOID STDMETHODCALLTYPE InternalAllocate(SIZE_T cb);
extern void STDMETHODCALLTYPE InternalFree(LPVOID pv);


MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));
    ThrowHrIfFailed(UnpackPackageFromStream(packUnpackOptions, validationOption, stream.Get(), utf8Destination));
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackageFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    IStream* stream,
    char* utf8Destination) noexcept try
{
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (stream != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IAppxFactory> factory;
    // We don't need to use the caller's heap here because we're not marshalling any strings
    // out to the caller.  So default to new / delete[] and be done with it!
    ThrowHrIfFailed(CoCreateAppxFactoryWithHeap(InternalAllocate, InternalFree, validationOption, &factory));

    MSIX::ComPtr<IAppxPackageReader> reader;
    ThrowHrIfFailed(factory->CreatePackageReader(stream, &reader));

    auto to = MSIX::ComPtr<IStorageObject>::Make<MSIX::DirectoryObject>(utf8Destination);
    reader.As<IPackage>()->Unpack(packUnpackOptions, to.Get());
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundle(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    char* utf8SourcePackage,
    char* utf8Destination) noexcept try
{
#ifdef BUNDLE_SUPPORT
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IStream> stream;
    ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));
    ThrowHrIfFailed(UnpackBundleFromStream(packUnpackOptions, validationOption, applicabilityOptions, stream.Get(), utf8Destination));
    return static_cast<HRESULT>(MSIX::Error::OK);
#else
    return static_cast<HRESULT>(MSIX::Error::NotSupported);
#endif
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IStream* stream,
    char* utf8Destination) noexcept try
{
#ifdef BUNDLE_SUPPORT
    ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
        (stream != nullptr && utf8Destination != nullptr), 
        "Invalid parameters"
    );

    MSIX::ComPtr<IAppxBundleFactory> factory;
    // We don't need to use the caller's heap here because we're not marshalling any strings
    // out to the caller.  So default to new / delete[] and be done with it!
    ThrowHrIfFailed(CoCreateAppxBundleFactoryWithHeap(InternalAllocate, InternalFree, validationOption, applicabilityOptions, &factory));

    MSIX::ComPtr<IAppxBundleReader> reader;
    ThrowHrIfFailed(factory->CreateBundleReader(stream, &reader));

    auto to = MSIX::ComPtr<IStorageObject>::Make<MSIX::DirectoryObject>(utf8Destination);
    reader.As<IPackage>()->Unpack(packUnpackOptions, to.Get());
    return static_cast<HRESULT>(MSIX::Error::OK);
#else
    return static_cast<HRESULT>(MSIX::Error::NotSupported);
#endif
} CATCH_RETURN();
