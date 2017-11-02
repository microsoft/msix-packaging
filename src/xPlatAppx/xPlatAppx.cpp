#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "FileStream.hpp"
#include "RangeStream.hpp"
#include "ZipObject.hpp"
#include "DirectoryObject.hpp"
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxPackageObject.hpp"
#include "AppxFactory.hpp"

#include <string>
#include <memory>


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
#endif

// on apple platforms, compile with -fvisibility=hidden
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
        ThrowHrIfFailed(CreateStreamOnFile(utf8SourcePackage, true, &stream));
        auto zipObject = std::make_unique<xPlat::ZipObject>(stream.Get());
        xPlat::AppxPackageObject appx(validationOption, std::move(zipObject));
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
        ThrowHrIfFailed(CreateStreamOnFile(utf8Destination, false, &stream));

        auto zipObject = std::make_unique<xPlat::ZipObject>(stream.Get());
        xPlat::AppxPackageObject appx(validationOption, std::move(zipObject));

        xPlat::DirectoryObject from(utf8FolderToPack);
        appx.Pack(packUnpackOptions, utf8CertificatePath, from);
        appx.CommitChanges();
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE ValidateAppxSignature(char* appx)
{
    return xPlat::ResultOf([&]() {
        xPlat::ComPtr<IStream> rawFile(new xPlat::FileStream(appx, xPlat::FileStream::Mode::READ));
        {
            xPlat::ZipObject zip(rawFile.Get());
            auto p7xStream = zip.GetFile("AppxSignature.p7x");
            std::vector<std::uint8_t> buffer(sizeof(_BLOBHEADER));

            ULONG cbRead;
            ThrowHrIfFailed(p7xStream->Read(reinterpret_cast<void*>(buffer.data()), buffer.size(), &cbRead));
            _BLOBHEADER *pblob = reinterpret_cast<_BLOBHEADER*>(buffer.data());

            ThrowErrorIfNot(xPlat::Error::AppxSignatureInvalid, (cbRead > sizeof(_BLOBHEADER) && pblob->headerId == SIGNATURE_ID), "Invalid signature");

            //auto rangeStream = std::make_unique<xPlat::RangeStream>(p7xStream, sizeof(P7xFileId), cbStream - sizeof(P7xFileId));
            //auto tempStream = std::make_unique<xPlat::FileStream>("e:\\temp\\temp.p7x", xPlat::FileStream::WRITE);
            //rangeStream->CopyTo(tempStream.get());
            //tempStream->Close();
        }
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream)
{
    return xPlat::ResultOf([&]() {
        xPlat::ComPtr<IStream> file(new xPlat::FileStream(utf8File, forRead ? xPlat::FileStream::Mode::READ : xPlat::FileStream::Mode::WRITE_UPDATE));
        *stream = file.Detach();
    });
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    return xPlat::ResultOf([&]() {
        xPlat::ComPtr<IAppxFactory> result(new xPlat::AppxFactory(memalloc, memfree));
        *appxFactory = result.Detach();
    });
}

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory)
{
    // #ifdef WIN32
    //     return CoCreateAppxFactoryWithHeap(CoTaskMemAlloc, CoTaskMemFree, validationOption, appxFactory);
    // #else
    //     return CoCreateAppxFactoryWithHeap(new, delete[], validationOption, AppxFactory);
    // #endif
    return static_cast<HRESULT>(xPlat::Error::NotImplemented);
}    