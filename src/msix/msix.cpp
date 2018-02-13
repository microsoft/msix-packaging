#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "RangeStream.hpp"
#include "ZipObject.hpp"
#include "DirectoryObject.hpp"
#include "UnicodeConversion.hpp"
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxPackageObject.hpp"
#include "AppxFactory.hpp"
#include "Log.hpp"

#include <string>
#include <memory>
#include <cstdlib>
#include <functional>

#ifdef WIN32
#include <Objbase.h>
#endif

//typedef void *BCRYPT_ALG_HANDLE;
//typedef void *BCRYPT_HASH_HANDLE;


#define MIN_DIGEST_COUNT          5           // All digests except code integrity
#define MAX_DIGEST_COUNT          6           // Including code integrity
#define ID_SIZE                   4           // IDs are 4 bytes
#define SHA_256_DIGEST_SIZE       32
#define SMALL_INDIRECT_DATA_SIZE  (ID_SIZE + (MIN_DIGEST_COUNT * (SHA_256_DIGEST_SIZE + ID_SIZE)))
#define LARGE_INDIRECT_DATA_SIZE  (ID_SIZE + (MAX_DIGEST_COUNT * (SHA_256_DIGEST_SIZE + ID_SIZE)))
#define CI_AND_SIG_DATA_SIZE      36
#define HEADER_BEGINNING_SIZE     38
#define FOUR_MB                   4194304     

//
// Magic Values
//
#define INDIRECT_DATA_ID          0x58504145  // EAPX
#define PACKAGE_HEADER_ID         0x48505845  // EXPH
#define BUNDLE_HEADER_ID          0x48425845  // EXBH
#define SIGNATURE_ID              0x58434B50  // PKCX
#define AXEH                      0x48455841  // Encrypted Appx Header
#define AXEF                      0x46455841  // Encrypted Appx Footer
#define AXEB                      0x42455841  // Encrypted Appx Block Map
#define AXPC                      0x43505841  // Encrypted Appx Package Content
#define AXBM                      0x4D425841  // Unencrypted Block Map
#define AXCI                      0x49435841  // Encrypted Appx Code Integrity
#define AXEH_IDX                  0
#define AXEF_IDX                  1
#define AXEB_IDX                  2
#define AXPC_IDX                  3
#define AXBM_IDX                  4
#define AXCI_IDX                  5

#define MAX_SUPPORTED_EAPPX_VERSION     0x0001000000000000  // 1.0.0.0

// Wrapper for BCRYPT hash handles 
#ifdef DISABLE
//typedef struct _SHA256_HANDLE
{
BCRYPT_ALG_HANDLE   hAlgorithm;
BCRYPT_HASH_HANDLE  hHash;
} SHA256_HANDLE, *PSHA256_HANDLE;
#endif
//
// The structure for relevant info for one hash.
//
struct INDIRECT_DATA_DIGEST
{
    std::uint32_t id;
    std::uint64_t start;
    std::uint64_t size;
    std::uint8_t value[SHA_256_DIGEST_SIZE];
} ;

//
// The structure that holds all hash data.
// 
struct EAPPX_INDIRECT_DATA
{
    std::uint32_t eappxId;
    std::uint8_t digestCount;
    INDIRECT_DATA_DIGEST digests[MAX_DIGEST_COUNT];
};

#ifdef WIN32
#include <pshpack1.h>
#endif 

struct _BLOBHEADER
{
    std::uint32_t headerId;
    std::uint16_t headerSize;
    std::uint64_t version;
    std::uint64_t footerOffset;
    std::uint64_t footerSize;
    std::uint64_t fileCount;

    std::uint64_t signatureOffset;
    std::uint16_t signatureCompressionType;
    std::uint32_t signatureUncompressedSize;
    std::uint32_t signatureCompressedSize;

    std::uint64_t codeIntegrityOffset;
    std::uint16_t codeIntegrityCompressionType;
    std::uint32_t codeIntegrityUncompressedSize;
    std::uint32_t codeIntegrityCompressedSize;
};

#ifdef WIN32
#include <poppack.h>
#else
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


MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination)
{
    return MSIX::ResultOf([&]() {
        ThrowErrorIfNot(MSIX::Error::InvalidParameter, 
            (utf8SourcePackage != nullptr && utf8Destination != nullptr), 
            "Invalid parameters"
        );

        MSIX::ComPtr<IAppxFactory> factory;
        // We don't need to use the caller's heap here because we're not marshalling any strings
        // out to the caller.  So default to new / delete[] and be done with it!
        ThrowHrIfFailed(CoCreateAppxFactoryWithHeap(InternalAllocate, InternalFree, validationOption, &factory));

        MSIX::ComPtr<IStream> stream;
        ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));

        MSIX::ComPtr<IAppxPackageReader> reader;
        ThrowHrIfFailed(factory->CreatePackageReader(stream.Get(), &reader));

        auto to = MSIX::ComPtr<IStorageObject>::Make<MSIX::DirectoryObject>(utf8Destination);
        reader.As<IPackage>()->Unpack(packUnpackOptions, to.Get());
    });
}

MSIX_API HRESULT STDMETHODCALLTYPE GetLogTextUTF8(COTASKMEMALLOC* memalloc, char** logText)
{
    return MSIX::ResultOf([&](){        
        ThrowErrorIf(MSIX::Error::InvalidParameter, (logText == nullptr || *logText != nullptr), "bad pointer" );
        std::size_t countBytes = sizeof(char)*(MSIX::Global::Log::Text().size()+1);
        *logText = reinterpret_cast<char*>(memalloc(countBytes));
        ThrowErrorIfNot(MSIX::Error::OutOfMemory, (*logText), "Allocation failed!");
        std::memset(reinterpret_cast<void*>(*logText), 0, countBytes);
        std::memcpy(reinterpret_cast<void*>(*logText),
                    reinterpret_cast<void*>(const_cast<char*>(MSIX::Global::Log::Text().c_str())),
                    countBytes - sizeof(char));
        MSIX::Global::Log::Clear();
    });
}

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream)
{
    return MSIX::ResultOf([&]() {
        MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
        *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(utf8File, mode).Detach();
    });
}

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFileUTF16(
    LPCWSTR utf16File,
    bool forRead,
    IStream** stream)
{
    return MSIX::ResultOf([&]() {
        MSIX::FileStream::Mode mode = forRead ? MSIX::FileStream::Mode::READ : MSIX::FileStream::Mode::WRITE_UPDATE;
        *stream = MSIX::ComPtr<IStream>::Make<MSIX::FileStream>(MSIX::utf16_to_utf8(utf16File), mode).Detach();
    });
}    

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    return MSIX::ResultOf([&]() {
        *appxFactory = MSIX::ComPtr<IAppxFactory>::Make<MSIX::AppxFactory>(validationOption, memalloc, memfree).Detach();
    });
}

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    #ifdef WIN32
        return CoCreateAppxFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, appxFactory);
    #else
        return static_cast<HRESULT>(MSIX::Error::NotSupported);
    #endif
}    