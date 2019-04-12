//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxFactory.hpp"
#include "Log.hpp"

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

LPVOID STDMETHODCALLTYPE InternalAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE InternalFree(LPVOID pv)        { std::free(pv); }

MSIX_API HRESULT STDMETHODCALLTYPE GetLogTextUTF8(COTASKMEMALLOC* memalloc, char** logText) noexcept try
{
    ThrowErrorIf(MSIX::Error::InvalidParameter, (logText == nullptr || *logText != nullptr), "bad pointer" );
    std::size_t countBytes = sizeof(char)*(MSIX::Global::Log::Text().size()+1);
    *logText = reinterpret_cast<char*>(memalloc(countBytes));
    ThrowErrorIfNot(MSIX::Error::OutOfMemory, (*logText), "Allocation failed!");
    std::memset(reinterpret_cast<void*>(*logText), 0, countBytes);
    std::memcpy(reinterpret_cast<void*>(*logText),
                reinterpret_cast<void*>(const_cast<char*>(MSIX::Global::Log::Text().c_str())),
                countBytes - sizeof(char));
    MSIX::Global::Log::Clear();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream) noexcept try
{
    MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
    *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf8File, mode).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFileUTF16(
    LPCWSTR utf16File,
    bool forRead,
    IStream** stream) noexcept try
{
    MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
    *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf16File, mode).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept try
{
    *appxFactory = MSIX::ComPtr<IAppxFactory>::Make<MSIX::AppxFactory>(validationOption, MSIX_APPLICABILITY_OPTION_FULL, memalloc, memfree).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
} CATCH_RETURN();

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept
{
    #ifdef WIN32
        return CoCreateAppxFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, appxFactory);
    #else
        return static_cast<HRESULT>(MSIX::Error::NotSupported);
    #endif
}

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept try
{
#ifdef BUNDLE_SUPPORT
    *appxBundleFactory = MSIX::ComPtr<IAppxBundleFactory>::Make<MSIX::AppxFactory>(validationOption, applicabilityOptions, memalloc, memfree).Detach();
    return static_cast<HRESULT>(MSIX::Error::OK);
#else
    return static_cast<HRESULT>(MSIX::Error::NotSupported);
#endif
} CATCH_RETURN();

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactory(
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept
{
    #if defined(WIN32) && defined(BUNDLE_SUPPORT)
        return CoCreateAppxBundleFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, applicabilityOptions, appxBundleFactory);
    #else
        return static_cast<HRESULT>(MSIX::Error::NotSupported);
    #endif
}
