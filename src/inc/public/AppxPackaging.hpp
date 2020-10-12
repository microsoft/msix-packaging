//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

/* File based on the header created by MIDL compiler version 8.01.0622 of AppxPackaging.idl */

#ifndef __appxpackaging_hpp__
#define __appxpackaging_hpp__

#include "MSIXWindows.hpp"

// Helper used for QueryInterface defines
template<typename T>
struct False
{
    static constexpr bool value = false;
};

template<typename Interface>
struct UuidOfImpl
{
    static_assert(False<Interface>::value, "Type doesn't have an IID");
};

#ifndef MSIX_INTERFACE
#define MSIX_INTERFACE(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    static constexpr const IID IID_##name  = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}; \
    template<>                                                                    \
    struct UuidOfImpl<name>                                                       \
    {                                                                             \
        static constexpr const IID& iid = IID_##name;                             \
    };
#endif // MSIX_INTERFACE

#ifdef WIN32
#include <AppxPackaging.h>

// This interfaces already have an IID_I* for windows. We still need its UuidOfImpl specialization
#ifndef SpecializeUuidOfImpl
#define SpecializeUuidOfImpl(Type)                    \
    template<>                                        \
    struct UuidOfImpl<Type>                           \
    {                                                 \
        static constexpr const IID& iid = IID_##Type; \
    };                                                \

SpecializeUuidOfImpl(IUnknown);
SpecializeUuidOfImpl(ISequentialStream);
SpecializeUuidOfImpl(IStream);
SpecializeUuidOfImpl(IAppxFactory);
SpecializeUuidOfImpl(IAppxPackageReader);
SpecializeUuidOfImpl(IAppxPackageWriter);
SpecializeUuidOfImpl(IAppxFile);
SpecializeUuidOfImpl(IAppxFilesEnumerator);
SpecializeUuidOfImpl(IAppxBlockMapReader);
SpecializeUuidOfImpl(IAppxBlockMapFile);
SpecializeUuidOfImpl(IAppxBlockMapFilesEnumerator);
SpecializeUuidOfImpl(IAppxBlockMapBlock);
SpecializeUuidOfImpl(IAppxBlockMapBlocksEnumerator);
SpecializeUuidOfImpl(IAppxManifestReader);
SpecializeUuidOfImpl(IAppxManifestReader2);
SpecializeUuidOfImpl(IAppxManifestReader3);
SpecializeUuidOfImpl(IAppxManifestReader4);
SpecializeUuidOfImpl(IAppxManifestReader5);
SpecializeUuidOfImpl(IAppxManifestPackageId);
SpecializeUuidOfImpl(IAppxManifestProperties);
SpecializeUuidOfImpl(IAppxManifestTargetDeviceFamiliesEnumerator);
SpecializeUuidOfImpl(IAppxManifestTargetDeviceFamily);
SpecializeUuidOfImpl(IAppxManifestPackageDependenciesEnumerator);
SpecializeUuidOfImpl(IAppxManifestPackageDependency);
SpecializeUuidOfImpl(IAppxManifestResourcesEnumerator);
SpecializeUuidOfImpl(IAppxManifestDeviceCapabilitiesEnumerator);
SpecializeUuidOfImpl(IAppxManifestCapabilitiesEnumerator);
SpecializeUuidOfImpl(IAppxManifestApplicationsEnumerator);
SpecializeUuidOfImpl(IAppxManifestApplication);
SpecializeUuidOfImpl(IAppxManifestQualifiedResourcesEnumerator);
SpecializeUuidOfImpl(IAppxManifestQualifiedResource);
SpecializeUuidOfImpl(IAppxBundleFactory);
SpecializeUuidOfImpl(IAppxBundleWriter);
SpecializeUuidOfImpl(IAppxBundleReader);
SpecializeUuidOfImpl(IAppxBundleManifestReader);
SpecializeUuidOfImpl(IAppxBundleManifestPackageInfoEnumerator);
SpecializeUuidOfImpl(IAppxBundleManifestPackageInfo);
SpecializeUuidOfImpl(IAppxPackageWriter3);
SpecializeUuidOfImpl(IAppxManifestOptionalPackageInfo);
SpecializeUuidOfImpl(IAppxManifestMainPackageDependenciesEnumerator);
SpecializeUuidOfImpl(IAppxManifestMainPackageDependency);
#endif

#else

// Interfaces
interface IUnknown;
interface ISequentialStream;
interface IStream;
interface IAppxFactory;
interface IAppxPackageReader;
interface IAppxPackageWriter;
interface IAppxFile;
interface IAppxFilesEnumerator;
interface IAppxBlockMapReader;
interface IAppxBlockMapFile;
interface IAppxBlockMapFilesEnumerator;
interface IAppxBlockMapBlock;
interface IAppxBlockMapBlocksEnumerator;
interface IAppxManifestReader;
interface IAppxManifestReader2;
interface IAppxManifestReader3;
interface IAppxManifestReader4;
interface IAppxManifestReader5;
interface IAppxManifestPackageId;
interface IAppxManifestProperties;
interface IAppxManifestTargetDeviceFamiliesEnumerator;
interface IAppxManifestTargetDeviceFamily;
interface IAppxManifestPackageDependenciesEnumerator;
interface IAppxManifestPackageDependency;
interface IAppxManifestResourcesEnumerator;
interface IAppxManifestDeviceCapabilitiesEnumerator;
interface IAppxManifestCapabilitiesEnumerator;
interface IAppxManifestApplicationsEnumerator;
interface IAppxManifestApplication;
interface IAppxManifestQualifiedResourcesEnumerator;
interface IAppxManifestQualifiedResource;
interface IAppxBundleFactory;
interface IAppxBundleWriter;
interface IAppxBundleReader;
interface IAppxBundleManifestReader;
interface IAppxBundleManifestPackageInfoEnumerator;
interface IAppxBundleManifestPackageInfo;
interface IAppxPackageWriter3;
interface IAppxManifestOptionalPackageInfo;
interface IAppxManifestMainPackageDependenciesEnumerator;
interface IAppxManifestMainPackageDependency;

extern "C"{

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

    // {00000000-0000-0000-C000-000000000046}
    MSIX_INTERFACE(IUnknown,0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
    interface IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ void **ppvObject) noexcept = 0;
        virtual ULONG STDMETHODCALLTYPE AddRef(void) noexcept = 0;
        virtual ULONG STDMETHODCALLTYPE Release( void) noexcept = 0;
    };
#endif  /* __IUnknown_INTERFACE_DEFINED__ */

#ifndef __ISequentialStream_INTERFACE_DEFINED__
#define __ISequentialStream_INTERFACE_DEFINED__

    // {0c733a30-2a1c-11ce-ade5-00aa0044773d}
    MSIX_INTERFACE(ISequentialStream,0x0c733a30,0x2a1c,0x11ce,0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d);
    interface ISequentialStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Read(
            /* [out] */  void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbRead) noexcept = 0;

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write(
            /* [in] */ const void *pv,
            /* [in]] */ ULONG cb,
            /* [out_opt] */ ULONG *pcbWritten) noexcept = 0;
    };
#endif  /* __ISequentialStream_INTERFACE_DEFINED__ */

#ifndef __IStream_INTERFACE_DEFINED__
#define __IStream_INTERFACE_DEFINED__

typedef struct tagSTATSTG
    {
    LPOLESTR pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
    }   STATSTG;

typedef
enum tagSTGTY
    {
        STGTY_STORAGE   = 1,
        STGTY_STREAM    = 2,
        STGTY_LOCKBYTES = 3,
        STGTY_PROPERTY  = 4
    }   STGTY;

typedef
enum tagSTREAM_SEEK
    {
        STREAM_SEEK_SET = 0,
        STREAM_SEEK_CUR = 1,
        STREAM_SEEK_END = 2
    }   STREAM_SEEK;

typedef
enum tagLOCKTYPE
    {
        LOCK_WRITE  = 1,
        LOCK_EXCLUSIVE  = 2,
        LOCK_ONLYONCE   = 4
    }   LOCKTYPE;

    // {0000000c-0000-0000-C000-000000000046}
    MSIX_INTERFACE(IStream,0x0000000c,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
    interface IStream : public ISequentialStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Seek(
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */  ULARGE_INTEGER *plibNewPosition) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE SetSize(
            /* [in] */ ULARGE_INTEGER libNewSize) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CopyTo(
            /* [unique][in] */  IStream *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER *pcbRead,
            /* [out] */ ULARGE_INTEGER *pcbWritten) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Commit(
            /* [in] */ DWORD grfCommitFlags) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Revert(void) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE LockRegion(
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Stat(
            /* [out] */ STATSTG *pstatstg,
            /* [in] */ DWORD grfStatFlag) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
            /* [out] */ IStream **ppstm) noexcept = 0;
    };
#endif  /* __IStream_INTERFACE_DEFINED__ */

#ifndef __IAppxFactory_INTERFACE_DEFINED__
#define __IAppxFactory_INTERFACE_DEFINED__

    typedef struct APPX_PACKAGE_SETTINGS
    {
        BOOL forceZip32;
        IUri *hashMethod;
    } 	APPX_PACKAGE_SETTINGS;

    typedef
        enum APPX_COMPRESSION_OPTION
    {
        APPX_COMPRESSION_OPTION_NONE = 0,
        APPX_COMPRESSION_OPTION_NORMAL = 1,
        APPX_COMPRESSION_OPTION_MAXIMUM = 2,
        APPX_COMPRESSION_OPTION_FAST = 3,
        APPX_COMPRESSION_OPTION_SUPERFAST = 4
    } 	APPX_COMPRESSION_OPTION;

    typedef
        enum APPX_FOOTPRINT_FILE_TYPE
    {
        APPX_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY = 3,
        APPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP = 4
    } 	APPX_FOOTPRINT_FILE_TYPE;

    typedef
        enum APPX_BUNDLE_FOOTPRINT_FILE_TYPE
    {
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_FIRST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_LAST = 2
    } 	APPX_BUNDLE_FOOTPRINT_FILE_TYPE;

    typedef
        enum APPX_CAPABILITIES
    {
        APPX_CAPABILITY_INTERNET_CLIENT = 0x1,
        APPX_CAPABILITY_INTERNET_CLIENT_SERVER = 0x2,
        APPX_CAPABILITY_PRIVATE_NETWORK_CLIENT_SERVER = 0x4,
        APPX_CAPABILITY_DOCUMENTS_LIBRARY = 0x8,
        APPX_CAPABILITY_PICTURES_LIBRARY = 0x10,
        APPX_CAPABILITY_VIDEOS_LIBRARY = 0x20,
        APPX_CAPABILITY_MUSIC_LIBRARY = 0x40,
        APPX_CAPABILITY_ENTERPRISE_AUTHENTICATION = 0x80,
        APPX_CAPABILITY_SHARED_USER_CERTIFICATES = 0x100,
        APPX_CAPABILITY_REMOVABLE_STORAGE = 0x200,
        APPX_CAPABILITY_APPOINTMENTS = 0x400,
        APPX_CAPABILITY_CONTACTS = 0x800
    } 	APPX_CAPABILITIES;

    typedef
        enum APPX_PACKAGE_ARCHITECTURE
    {
        APPX_PACKAGE_ARCHITECTURE_X86 = 0,
        APPX_PACKAGE_ARCHITECTURE_ARM = 5,
        APPX_PACKAGE_ARCHITECTURE_X64 = 9,
        APPX_PACKAGE_ARCHITECTURE_NEUTRAL = 11,
        APPX_PACKAGE_ARCHITECTURE_ARM64 = 12
    } 	APPX_PACKAGE_ARCHITECTURE;

    typedef
        enum APPX_PACKAGE_ARCHITECTURE2
    {
        APPX_PACKAGE_ARCHITECTURE2_X86 = 0,
        APPX_PACKAGE_ARCHITECTURE2_ARM = 5,
        APPX_PACKAGE_ARCHITECTURE2_X64 = 9,
        APPX_PACKAGE_ARCHITECTURE2_NEUTRAL = 11,
        APPX_PACKAGE_ARCHITECTURE2_ARM64 = 12,
        APPX_PACKAGE_ARCHITECTURE2_X86_ON_ARM64 = 14,
        APPX_PACKAGE_ARCHITECTURE2_UNKNOWN = 0xffff
    } 	APPX_PACKAGE_ARCHITECTURE2;

    typedef
        enum APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION = 0,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE = 1
    } 	APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE;

    typedef
        enum DX_FEATURE_LEVEL
    {
        DX_FEATURE_LEVEL_UNSPECIFIED = 0,
        DX_FEATURE_LEVEL_9 = 1,
        DX_FEATURE_LEVEL_10 = 2,
        DX_FEATURE_LEVEL_11 = 3
    } 	DX_FEATURE_LEVEL;

    typedef
        enum APPX_CAPABILITY_CLASS_TYPE
    {
        APPX_CAPABILITY_CLASS_DEFAULT = 0,
        APPX_CAPABILITY_CLASS_GENERAL = 0x1,
        APPX_CAPABILITY_CLASS_RESTRICTED = 0x2,
        APPX_CAPABILITY_CLASS_WINDOWS = 0x4,
        APPX_CAPABILITY_CLASS_ALL = 0x7,
        APPX_CAPABILITY_CLASS_CUSTOM = 0x8
    } 	APPX_CAPABILITY_CLASS_TYPE;

    // {beb94909-e451-438b-b5a7-d79e767b75d8}
    MSIX_INTERFACE(IAppxFactory,0xbeb94909,0xe451,0x438b,0xb5,0xa7,0xd7,0x9e,0x76,0x7b,0x75,0xd8);
    interface IAppxFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePackageWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */  APPX_PACKAGE_SETTINGS *settings,
            /* [retval][out] */  IAppxPackageWriter **packageWriter) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreatePackageReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxPackageReader **packageReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateManifestReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxManifestReader **manifestReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBlockMapReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(
            /* [in] */  IStream *blockMapStream,
            /* [in] */  LPCWSTR signatureFileName,
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) noexcept = 0;
    };
#endif  /* __IAppxFactory_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageReader_INTERFACE_DEFINED__
#define __IAppxPackageReader_INTERFACE_DEFINED__

    // {b5c49650-99bc-481c-9a34-}
    MSIX_INTERFACE(IAppxPackageReader,0xb5c49650,0x99bc,0x481c,0x9a,0x34,0x3d,0x53,0xa4,0x10,0x67,0x08);
    interface IAppxPackageReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlockMap(
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFootprintFile(
            /* [in] */ APPX_FOOTPRINT_FILE_TYPE type,
            /* [retval][out] */  IAppxFile **file) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadFile(
            /* [string][in] */  LPCWSTR fileName,
            /* [retval][out] */  IAppxFile **file) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadFiles(
            /* [retval][out] */  IAppxFilesEnumerator **filesEnumerator) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetManifest(
            /* [retval][out] */  IAppxManifestReader **manifestReader) noexcept = 0;
    };
#endif 	/* __IAppxPackageReader_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageWriter_INTERFACE_DEFINED__
#define __IAppxPackageWriter_INTERFACE_DEFINED__

    // {9099e33b-246f-41e4-881a-008eb613f858}
    MSIX_INTERFACE(IAppxPackageWriter,0x9099e33b,0x246f,0x41e4,0x88,0x1a,0x00,0x8e,0xb6,0x13,0xf8,0x58);
    interface IAppxPackageWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFile(
            /* [string][in] */  LPCWSTR fileName,
            /* [string][in] */  LPCWSTR contentType,
            /* [in] */ APPX_COMPRESSION_OPTION compressionOption,
            /* [in] */  IStream *inputStream) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Close(
            /* [in] */  IStream *manifest) noexcept = 0;

    };
#endif 	/* __IAppxPackageWriter_INTERFACE_DEFINED__ */

#ifndef __IAppxFile_INTERFACE_DEFINED__
#define __IAppxFile_INTERFACE_DEFINED__

    // {91df827b-94fd-468f-827b-57f41b2f6f2e}
    MSIX_INTERFACE(IAppxFile,0x91df827b,0x94fd,0x468f,0x82,0x7b,0x57,0xf4,0x1b,0x2f,0x6f,0x2e);
    interface IAppxFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCompressionOption(
            /* [retval][out] */  APPX_COMPRESSION_OPTION *compressionOption) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetContentType(
            /* [retval][string][out] */ LPWSTR *contentType) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *fileName) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSize(
            /* [retval][out] */  UINT64 *size) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **stream) noexcept = 0;
    };
#endif 	/* __IAppxFile_INTERFACE_DEFINED__ */

#ifndef __IAppxFilesEnumerator_INTERFACE_DEFINED__
#define __IAppxFilesEnumerator_INTERFACE_DEFINED__

    // {f007eeaf-9831-411c-9847-917cdc62d1fe}
    MSIX_INTERFACE(IAppxFilesEnumerator,0xf007eeaf,0x9831,0x411c,0x98,0x47,0x91,0x7c,0xdc,0x62,0xd1,0xfe);
    interface IAppxFilesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxFile **file) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;

    };
#endif 	/* __IAppxFilesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapReader_INTERFACE_DEFINED__
#define __IAppxBlockMapReader_INTERFACE_DEFINED__

    // {5efec991-bca3-42d1-9ec2-e92d609ec22a}
    MSIX_INTERFACE(IAppxBlockMapReader,0x5efec991,0xbca3,0x42d1,0x9e,0xc2,0xe9,0x2d,0x60,0x9e,0xc2,0x2a);
    interface IAppxBlockMapReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFile(
            /* [string][in] */  LPCWSTR filename,
            /* [retval][out] */  IAppxBlockMapFile **file) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFiles(
            /* [retval][out] */  IAppxBlockMapFilesEnumerator **enumerator) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHashMethod(
            /* [retval][out] */  IUri **hashMethod) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **blockMapStream) noexcept = 0;
    };
#endif 	/* __IAppxBlockMapReader_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapFile_INTERFACE_DEFINED__
#define __IAppxBlockMapFile_INTERFACE_DEFINED__

    // {277672ac-4f63-42c1-8abc-beae3600eb59}
    MSIX_INTERFACE(IAppxBlockMapFile,0x277672ac,0x4f63,0x42c1,0x8a,0xbc,0xbe,0xae,0x36,0x00,0xeb,0x59);
    interface IAppxBlockMapFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlocks(
            /* [retval][out] */  IAppxBlockMapBlocksEnumerator **blocks) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize(
            /* [retval][out] */  UINT32 *lfhSize) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetUncompressedSize(
            /* [retval][out] */  UINT64 *size) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE ValidateFileHash(
            /* [in] */  IStream *fileStream,
            /* [retval][out] */  BOOL *isValid) noexcept = 0;

    };
#endif 	/* __IAppxBlockMapFile_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__
#define __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__

    // {02b856a2-4262-4070-bacb-1a8cbbc42305}
    MSIX_INTERFACE(IAppxBlockMapFilesEnumerator,0x02b856a2,0x4262,0x4070,0xba,0xcb,0x1a,0x8c,0xbb,0xc4,0x23,0x05);
    interface IAppxBlockMapFilesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBlockMapFile **file) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;
    };
#endif 	/* __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapBlock_INTERFACE_DEFINED__
#define __IAppxBlockMapBlock_INTERFACE_DEFINED__

    // {75cf3930-3244-4fe0-a8c8-e0bcb270b889}
    MSIX_INTERFACE(IAppxBlockMapBlock,0x75cf3930,0x3244,0x4fe0,0xa8,0xc8,0xe0,0xbc,0xb2,0x70,0xb8,0x89);
    interface IAppxBlockMapBlock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHash(
            /* [out] */  UINT32 *bufferSize,
            /* [retval][size_is][size_is][out] */  BYTE **buffer) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCompressedSize(
            /* [retval][out] */  UINT32 *size)  noexcept = 0;
    };
#endif 	/* __IAppxBlockMapBlock_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__
#define __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__

    // {6b429b5b-36ef-479e-b9eb-0c1482b49e16}
    MSIX_INTERFACE(IAppxBlockMapBlocksEnumerator,0x6b429b5b,0x36ef,0x479e,0xb9,0xeb,0x0c,0x14,0x82,0xb4,0x9e,0x16);
    interface IAppxBlockMapBlocksEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBlockMapBlock **block) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader_INTERFACE_DEFINED__
#define __IAppxManifestReader_INTERFACE_DEFINED__

    // {4e1bd148-55a0-4480-a3d1-15544710637c}
    MSIX_INTERFACE(IAppxManifestReader,0x4e1bd148,0x55a0,0x4480,0xa3,0xd1,0x15,0x54,0x47,0x10,0x63,0x7c);
    interface IAppxManifestReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetProperties(
            /* [retval][out] */  IAppxManifestProperties **packageProperties) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageDependencies(
            /* [retval][out] */  IAppxManifestPackageDependenciesEnumerator **dependencies) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCapabilities(
            /* [retval][out] */  APPX_CAPABILITIES *capabilities) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResources(
            /* [retval][out] */  IAppxManifestResourcesEnumerator **resources) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDeviceCapabilities(
            /* [retval][out] */  IAppxManifestDeviceCapabilitiesEnumerator **deviceCapabilities) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPrerequisite(
            /* [string][in] */  LPCWSTR name,
            /* [retval][out] */  UINT64 *value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetApplications(
            /* [retval][out] */  IAppxManifestApplicationsEnumerator **applications) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **manifestStream) noexcept = 0;
    };
#endif 	/* __IAppxManifestReader_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader2_INTERFACE_DEFINED__
#define __IAppxManifestReader2_INTERFACE_DEFINED__

    // {d06f67bc-b31d-4eba-a8af-638e73e77b4d}
    MSIX_INTERFACE(IAppxManifestReader2,0xd06f67bc,0xb31d,0x4eba,0xa8,0xaf,0x63,0x8e,0x73,0xe7,0x7b,0x4d);
    interface IAppxManifestReader2 : public IAppxManifestReader
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetQualifiedResources(
            /* [retval][out] */  IAppxManifestQualifiedResourcesEnumerator **resources) noexcept = 0;

    };
#endif 	/* __IAppxManifestReader2_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader3_INTERFACE_DEFINED__
#define __IAppxManifestReader3_INTERFACE_DEFINED__

    // {c43825ab-69b7-400a-9709-cc37f5a72d24}
    MSIX_INTERFACE(IAppxManifestReader3,0xc43825ab,0x69b7,0x400a,0x97,0x09,0xcc,0x37,0xf5,0xa7,0x2d,0x24);
    interface IAppxManifestReader3 : public IAppxManifestReader2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCapabilitiesByCapabilityClass(
            /* [in] */ APPX_CAPABILITY_CLASS_TYPE capabilityClass,
            /* [retval][out] */  IAppxManifestCapabilitiesEnumerator **capabilities) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetTargetDeviceFamilies(
            /* [retval][out] */  IAppxManifestTargetDeviceFamiliesEnumerator **targetDeviceFamilies) noexcept = 0;
    };
#endif 	/* __IAppxManifestReader3_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader4_INTERFACE_DEFINED__
#define __IAppxManifestReader4_INTERFACE_DEFINED__

    // {4579bb7c-741d-4161-b5a1-47bd3b78ad9b}
    MSIX_INTERFACE(IAppxManifestReader4, 0x4579bb7c, 0x741d, 0x4161, 0xb5, 0xa1, 0x47, 0xbd, 0x3b, 0x78, 0xad, 0x9b);
    interface IAppxManifestReader4 : public IAppxManifestReader3
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOptionalPackageInfo(
            /* [retval][out] */  IAppxManifestOptionalPackageInfo **optionalPackageInfo) noexcept = 0;
    };
#endif 	/* __IAppxManifestReader4_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader5_INTERFACE_DEFINED__
#define __IAppxManifestReader5_INTERFACE_DEFINED__

    // {8d7ae132-a690-4c00-b75a-6aae1feaac80}
    MSIX_INTERFACE(IAppxManifestReader5, 0x8d7ae132, 0xa690, 0x4c00, 0xb7, 0x5a, 0x6a, 0xae, 0x1f, 0xea, 0xac, 0x80);
    interface IAppxManifestReader5 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMainPackageDependencies(
            /* [retval][out] */  IAppxManifestMainPackageDependenciesEnumerator **mainPackageDependencies) noexcept = 0;
    };
#endif 	/* __IAppxManifestReader5_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageId_INTERFACE_DEFINED__
#define __IAppxManifestPackageId_INTERFACE_DEFINED__

    // {283ce2d7-7153-4a91-9649-7a0f7240945f}
    MSIX_INTERFACE(IAppxManifestPackageId,0x283ce2d7,0x7153,0x4a91,0x96,0x49,0x7a,0x0f,0x72,0x40,0x94,0x5f);
    interface IAppxManifestPackageId : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetArchitecture(
            /* [retval][out] */  APPX_PACKAGE_ARCHITECTURE *architecture) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPWSTR *publisher) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetVersion(
            /* [retval][out] */  UINT64 *packageVersion) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResourceId(
            /* [retval][string][out] */ LPWSTR *resourceId) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE ComparePublisher(
            /* [string][in] */  LPCWSTR other,
            /* [retval][out] */  BOOL *isSame) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFullName(
            /* [retval][string][out] */ LPWSTR *packageFullName) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */ LPWSTR *packageFamilyName) noexcept = 0;
    };
#endif 	/* __IAppxManifestPackageId_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestProperties_INTERFACE_DEFINED__
#define __IAppxManifestProperties_INTERFACE_DEFINED__

    // {03faf64d-f26f-4b2c-aaf7-8fe7789b8bca}
    MSIX_INTERFACE(IAppxManifestProperties,0x03faf64d,0xf26f,0x4b2c,0xaa,0xf7,0x8f,0xe7,0x78,0x9b,0x8b,0xca);
    interface IAppxManifestProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBoolValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][out] */  BOOL *value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][string][out] */ LPWSTR *value) noexcept = 0;
    };
#endif 	/* __IAppxManifestProperties_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__

    // {36537f36-27a4-4788-88c0-733819575017}
    MSIX_INTERFACE(IAppxManifestTargetDeviceFamiliesEnumerator,0x36537F36,0x27A4,0x4788,0x88,0xC0,0x73,0x38,0x19,0x57,0x50,0x17);
    interface IAppxManifestTargetDeviceFamiliesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestTargetDeviceFamily **targetDeviceFamily) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__
#define __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__

    // {9091b09b-c8d5-4f31-8687-a338259faefb}
    MSIX_INTERFACE(IAppxManifestTargetDeviceFamily,0x9091B09B,0xC8D5,0x4F31,0x86,0x87,0xA3,0x38,0x25,0x9F,0xAE,0xFB);
    interface IAppxManifestTargetDeviceFamily : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMinVersion(
            /* [retval][out] */  UINT64 *minVersion) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMaxVersionTested(
            /* [retval][out] */  UINT64 *maxVersionTested) noexcept = 0;

    };
#endif 	/* __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__

    // {b43bbcf9-65a6-42dd-bac0-8c6741e7f5a4}
    MSIX_INTERFACE(IAppxManifestPackageDependenciesEnumerator,0xb43bbcf9,0x65a6,0x42dd,0xba,0xc0,0x8c,0x67,0x41,0xe7,0xf5,0xa4);
    interface IAppxManifestPackageDependenciesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestPackageDependency **dependency) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__

    // {de4dfbbd-881a-48bb-858c-d6f2baeae6ed}
    MSIX_INTERFACE(IAppxManifestResourcesEnumerator,0xde4dfbbd,0x881a,0x48bb,0x85,0x8c,0xd6,0xf2,0xba,0xea,0xe6,0xed);
    interface IAppxManifestResourcesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *resource) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__

    // {30204541-427b-4a1c-bacf-655bf463a540}
    MSIX_INTERFACE(IAppxManifestDeviceCapabilitiesEnumerator,0x30204541,0x427b,0x4a1c,0xba,0xcf,0x65,0x5b,0xf4,0x63,0xa5,0x40);
    interface IAppxManifestDeviceCapabilitiesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *deviceCapability) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;

    };
#endif 	/* __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__

    // {11d22258-f470-42c1-b291-8361c5437e41}
    MSIX_INTERFACE(IAppxManifestCapabilitiesEnumerator,0x11D22258,0xF470,0x42C1,0xB2,0x91,0x83,0x61,0xC5,0x43,0x7E,0x41);
    interface IAppxManifestCapabilitiesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *capability) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;

    };
#endif 	/* __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__

    // {9eb8a55a-f04b-4d0d-808d-686185d4847a}
    MSIX_INTERFACE(IAppxManifestApplicationsEnumerator,0x9eb8a55a,0xf04b,0x4d0d,0x80,0x8d,0x68,0x61,0x85,0xd4,0x84,0x7a);
    interface IAppxManifestApplicationsEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestApplication **application) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;

    };
#endif 	/* __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestApplication_INTERFACE_DEFINED__
#define __IAppxManifestApplication_INTERFACE_DEFINED__

    // {5da89bf4-3773-46be-b650-7e744863b7e8}
    MSIX_INTERFACE(IAppxManifestApplication,0x5da89bf4,0x3773,0x46be,0xb6,0x50,0x7e,0x74,0x48,0x63,0xb7,0xe8);
    interface IAppxManifestApplication : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][string][out] */  LPWSTR *value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(
            /* [retval][string][out] */  LPWSTR *appUserModelId) noexcept = 0;
    };
#endif 	/* __IAppxManifestApplication_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__

    // {8ef6adfe-3762-4a8f-9373-2fc5d444c8d2}
    MSIX_INTERFACE(IAppxManifestQualifiedResourcesEnumerator,0x8ef6adfe,0x3762,0x4a8f,0x93,0x73,0x2f,0xc5,0xd4,0x44,0xc8,0xd2);
    interface IAppxManifestQualifiedResourcesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestQualifiedResource **resource) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestQualifiedResource_INTERFACE_DEFINED__
#define __IAppxManifestQualifiedResource_INTERFACE_DEFINED__

    // {3b53a497-3c5c-48d1-9ea3-bb7eac8cd7d4}
    MSIX_INTERFACE(IAppxManifestQualifiedResource,0x3b53a497,0x3c5c,0x48d1,0x9e,0xa3,0xbb,0x7e,0xac,0x8c,0xd7,0xd4);
    interface IAppxManifestQualifiedResource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLanguage(
            /* [retval][string][out] */  LPWSTR *language) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetScale(
            /* [retval][out] */  UINT32 *scale) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDXFeatureLevel(
            /* [retval][out] */  DX_FEATURE_LEVEL *dxFeatureLevel) noexcept = 0;

    };
#endif 	/* __IAppxManifestQualifiedResource_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleFactory_INTERFACE_DEFINED__
#define __IAppxBundleFactory_INTERFACE_DEFINED__

    // {bba65864-965f-4a5f-855f-f074bdbf3a7b}
    MSIX_INTERFACE(IAppxBundleFactory,0xbba65864,0x965f,0x4a5f,0x85,0x5f,0xf0,0x74,0xbd,0xbf,0x3a,0x7b);
    interface IAppxBundleFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateBundleWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */ UINT64 bundleVersion,
            /* [retval][out] */  IAppxBundleWriter **bundleWriter) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBundleReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBundleReader **bundleReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBundleManifestReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBundleManifestReader **manifestReader) noexcept = 0;

    };
#endif 	/* __IAppxBundleFactory_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleWriter_INTERFACE_DEFINED__
#define __IAppxBundleWriter_INTERFACE_DEFINED__

    // {ec446fe8-bfec-4c64-ab4f-49f038f0c6d2}
    MSIX_INTERFACE(IAppxBundleWriter,0xec446fe8,0xbfec,0x4c64,0xab,0x4f,0x49,0xf0,0x38,0xf0,0xc6,0xd2);
    interface IAppxBundleWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadPackage(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *packageStream) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Close(void) noexcept = 0;

    };
#endif 	/* __IAppxBundleWriter_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleReader_INTERFACE_DEFINED__
#define __IAppxBundleReader_INTERFACE_DEFINED__

    // {dd75b8c0-ba76-43b0-ae0f-68656a1dc5c8}
    MSIX_INTERFACE(IAppxBundleReader,0xdd75b8c0,0xba76,0x43b0,0xae,0x0f,0x68,0x65,0x6a,0x1d,0xc5,0xc8);
    interface IAppxBundleReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFootprintFile(
            /* [in] */ APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType,
            /* [retval][out] */  IAppxFile **footprintFile) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetBlockMap(
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetManifest(
            /* [retval][out] */  IAppxBundleManifestReader **manifestReader) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadPackages(
            /* [retval][out] */  IAppxFilesEnumerator **payloadPackages) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadPackage(
            /* [string][in] */  LPCWSTR fileName,
            /* [retval][out] */  IAppxFile **payloadPackage) noexcept = 0;
    };
#endif 	/* __IAppxBundleReader_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestReader_INTERFACE_DEFINED__
#define __IAppxBundleManifestReader_INTERFACE_DEFINED__

    // {cf0ebbc1-cc99-4106-91eb-e67462e04fb0}
    MSIX_INTERFACE(IAppxBundleManifestReader,0xcf0ebbc1,0xcc99,0x4106,0x91,0xeb,0xe6,0x74,0x62,0xe0,0x4f,0xb0);
    interface IAppxBundleManifestReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageInfoItems(
            /* [retval][out] */  IAppxBundleManifestPackageInfoEnumerator **packageInfoItems) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **manifestStream) noexcept = 0;
    };
#endif 	/* __IAppxBundleManifestReader_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageDependency_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependency_INTERFACE_DEFINED__

    // {e4946b59-733e-43f0-a724-3bde4c1285a0}
    MSIX_INTERFACE(IAppxManifestPackageDependency,0xe4946b59,0x733e,0x43f0,0xa7,0x24,0x3b,0xde,0x4c,0x12,0x85,0xa0);
    interface IAppxManifestPackageDependency : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPWSTR *publisher) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMinVersion(
            /* [retval][out] */  UINT64 *minVersion) noexcept = 0;
    };
#endif 	/* __IAppxManifestPackageDependency_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__

    // {f9b856ee-49a6-4e19-b2b0-6a2406d63a32}
    MSIX_INTERFACE(IAppxBundleManifestPackageInfoEnumerator,0xf9b856ee,0x49a6,0x4e19,0xb2,0xb0,0x6a,0x24,0x06,0xd6,0x3a,0x32);
    interface IAppxBundleManifestPackageInfoEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBundleManifestPackageInfo **packageInfo) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__

    // {54cd06c1-268f-40bb-8ed2-757a9ebaec8d}
    MSIX_INTERFACE(IAppxBundleManifestPackageInfo,0x54cd06c1,0x268f,0x40bb,0x8e,0xd2,0x75,0x7a,0x9e,0xba,0xec,0x8d);
    interface IAppxBundleManifestPackageInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageType(
            /* [retval][out] */  APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE *packageType) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFileName(
            /* [retval][string][out] */  LPWSTR *fileName) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetOffset(
            /* [retval][out] */  UINT64 *offset) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSize(
            /* [retval][out] */  UINT64 *size) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResources(
            /* [retval][out] */  IAppxManifestQualifiedResourcesEnumerator **resources) noexcept = 0;
    };
#endif 	/* __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageWriter3_INTERFACE_DEFINED__
#define __IAppxPackageWriter3_INTERFACE_DEFINED__

    typedef struct APPX_PACKAGE_WRITER_PAYLOAD_STREAM
    {
        IStream* inputStream;
        LPCWSTR fileName;
        LPCWSTR contentType;
        APPX_COMPRESSION_OPTION compressionOption;
    } 	APPX_PACKAGE_WRITER_PAYLOAD_STREAM;

    // {a83aacd3-41c0-4501-b8a3-74164f50b2fd}
    MSIX_INTERFACE(IAppxPackageWriter3, 0xa83aacd3,0x41c0,0x4501,0xb8,0xa3,0x74,0x16,0x4f,0x50,0xb2,0xfd);
    interface IAppxPackageWriter3 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFiles( 
            /* [in] */ UINT32 fileCount,
            /* [size_is][in] */ APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles,
            /* [in] */ UINT64 memoryLimit) = 0;
    };
#endif 	/* __IAppxPackageWriter3_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__
#define __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__

    // {2634847d-5b5d-4fe5-a243-002ff95edc7e}
    MSIX_INTERFACE(IAppxManifestOptionalPackageInfo, 0x2634847d, 0x5b5d, 0x4fe5, 0xa2, 0x43, 0x00, 0x2f, 0xf9, 0x5e, 0xdc, 0x7e);
    interface IAppxManifestOptionalPackageInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIsOptionalPackage(
            /* [retval][out] */  BOOL *isOptionalPackage) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMainPackageName(
            /* [retval][string][out] */  LPWSTR *mainPackageName) noexcept = 0;
    };
#endif 	/* __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__

    // {a99c4f00-51d2-4f0f-ba46-7ed5255ebdff}
    MSIX_INTERFACE(IAppxManifestMainPackageDependenciesEnumerator, 0xa99c4f00, 0x51d2, 0x4f0f, 0xba, 0x46, 0x7e, 0xd5, 0x25, 0x5e, 0xbd, 0xff);
    interface IAppxManifestMainPackageDependenciesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestMainPackageDependency **mainPackageDependency) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif 	/* __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__
#define __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__

    // {05d0611c-bc29-46d5-97e2-84b9c79bd8ae}
    MSIX_INTERFACE(IAppxManifestMainPackageDependency, 0x05d0611c, 0xbc29, 0x46d5, 0x97, 0xe2, 0x84, 0xb9, 0xc7, 0x9b, 0xd8, 0xae);
    interface IAppxManifestMainPackageDependency : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */  LPWSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */  LPWSTR *publisher) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */  LPWSTR *packageFamilyName) noexcept = 0;
    };
#endif 	/* __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__ */


} // extern "C"
#endif // #ifdef WIN32

// MSIX specific

// MSIX public interfaces
interface IMsixDocumentElement;
interface IMsixElement;
interface IMsixElementEnumerator;
interface IMsixFactoryOverrides;
interface IMsixStreamFactory;
interface IMsixApplicabilityLanguagesEnumerator;

#ifndef __IMsixDocumentElement_INTERFACE_DEFINED__
#define __IMsixDocumentElement_INTERFACE_DEFINED__

    // {e8900e0e-1dfd-4728-8352-aadaebbf0065}
    MSIX_INTERFACE(IMsixDocumentElement,0xe8900e0e,0x1dfd,0x4728,0x83,0x52,0xaa,0xda,0xeb,0xbf,0x00,0x65);
    interface IMsixDocumentElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentElement(
            /* [retval][out] */ IMsixElement** documentElement) noexcept = 0;
    };
#endif  /* __IMsixDocumentElement_INTERFACE_DEFINED__ */

#ifndef __IMsixElement_INTERFACE_DEFINED__
#define __IMsixElement_INTERFACE_DEFINED__

    // {5b6786ff-6145-4f0e-b8c9-8e03aacb60d0}
    MSIX_INTERFACE(IMsixElement,0x5b6786ff,0x6145,0x4f0e,0xb8,0xc9,0x8e,0x03,0xaa,0xcb,0x60,0xd0);
    interface IMsixElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAttributeValue(
            /* [in] */ LPCWSTR name,
            /* [retval][string][out] */ LPWSTR* value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetText(
            /* [retval][string][out] */ LPWSTR* value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetElements(
            /* [in] */ LPCWSTR xpath,
            /* [retval][out] */ IMsixElementEnumerator** elements) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAttributeValueUtf8(
            /* [in] */ LPCSTR name,
            /* [retval][string][out] */ LPSTR* value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetTextUtf8(
            /* [retval][string][out] */ LPSTR* value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetElementsUtf8(
            /* [in] */ LPCSTR xpath,
            /* [retval][out] */ IMsixElementEnumerator** elements) noexcept= 0;
    };
#endif  /* __IMsixElement_INTERFACE_DEFINED__ */

#ifndef __IMsixElementEnumerator_INTERFACE_DEFINED__
#define __IMsixElementEnumerator_INTERFACE_DEFINED__

    // {7e7ea105-a4f9-4c12-9efa-98ef1895418a}
    MSIX_INTERFACE(IMsixElementEnumerator,0x7e7ea105,0xa4f9,0x4c12,0x9e,0xfa,0x98,0xef,0x18,0x95,0x41,0x8a);
    interface IMsixElementEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */ IMsixElement** element) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */ BOOL* hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */ BOOL* hasNext) noexcept = 0;
    };
#endif  /* __IMsixElementEnumerator_INTERFACE_DEFINED__ */

#ifndef __IMsixFactoryOverrides_INTERFACE_DEFINED__
#define __IMsixFactoryOverrides_INTERFACE_DEFINED__

    typedef
        enum MSIX_FACTORY_EXTENSION
    {
        MSIX_FACTORY_EXTENSION_STREAM_FACTORY = 0x1,
        MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES = 0x2,
    } 	MSIX_FACTORY_EXTENSION;

    // {0acedbdb-57cd-4aca-8cee-33fa52394316}
    MSIX_INTERFACE(IMsixFactoryOverrides,0x0acedbdb,0x57cd,0x4aca,0x8c,0xee,0x33,0xfa,0x52,0x39,0x43,0x16);
    interface IMsixFactoryOverrides : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SpecifyExtension(
            /* [in] */ MSIX_FACTORY_EXTENSION name,
            /* [retval][out] */ IUnknown* extension) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCurrentSpecifiedExtension(
            /* [in] */ MSIX_FACTORY_EXTENSION name,
            /* [retval][out] */ IUnknown** extension) noexcept = 0;
    };
#endif  /* __IMsixFactoryOverrides_INTERFACE_DEFINED__ */

#ifndef __IMsixStreamFactory_INTERFACE_DEFINED__
#define __IMsixStreamFactory_INTERFACE_DEFINED__

    // {c74f4821-3b82-4ad5-98ea-3d52681aff56}
    MSIX_INTERFACE(IMsixStreamFactory,0xc74f4821,0x3b82,0x4ad5,0x98,0xea,0x3d,0x52,0x68,0x1a,0xff,0x56);
    interface IMsixStreamFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStreamOnRelativePath(
            /* [in] */ LPCWSTR relativePath,
            /* [retval][out] */ IStream** stream) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateStreamOnRelativePathUtf8(
            /* [in] */ LPCSTR relativePath,
            /* [retval][out] */ IStream** stream) noexcept = 0;
    };
#endif  /* __IMsixStreamFactory_INTERFACE_DEFINED__ */

#ifndef __IMsixApplicabilityLanguagesEnumerator_INTERFACE_DEFINED__
#define __IMsixApplicabilityLanguagesEnumerator_INTERFACE_DEFINED__

    // {bfc4655a-be7a-456a-bc4e-2af9481e8432}
    MSIX_INTERFACE(IMsixApplicabilityLanguagesEnumerator,0xbfc4655a,0xbe7a,0x456a,0xbc,0x4e,0x2a,0xf9,0x48,0x1e,0x84,0x32);
    interface IMsixApplicabilityLanguagesEnumerator : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPCSTR *bcp47Language) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) noexcept = 0;
    };
#endif  /* __IMsixApplicabilityLanguagesEnumerator_INTERFACE_DEFINED__ */

// Specific to MSIX SDK. UTF8 variant of AppxPackaging interfaces
interface IAppxBlockMapFileUtf8;
interface IAppxBlockMapReaderUtf8;
interface IAppxBundleManifestPackageInfoUtf8;
interface IAppxBundleReaderUtf8;
interface IAppxFactoryUtf8;
interface IAppxFileUtf8;
interface IAppxManifestApplicationUtf8;
interface IAppxManifestPackageDependencyUtf8;
interface IAppxManifestPackageIdUtf8;
interface IAppxManifestPropertiesUtf8;
interface IAppxManifestQualifiedResourceUtf8;
interface IAppxManifestCapabilitiesEnumeratorUtf8;
interface IAppxManifestResourcesEnumeratorUtf8;
interface IAppxManifestTargetDeviceFamilyUtf8;
interface IAppxPackageReaderUtf8;
interface IAppxPackageWriterUtf8;
interface IAppxPackageWriter3Utf8;
interface IAppxManifestOptionalPackageInfoUtf8;
interface IAppxManifestMainPackageDependencyUtf8;

#ifndef __IAppxBlockMapFileUtf8_INTERFACE_DEFINED__
#define __IAppxBlockMapFileUtf8_INTERFACE_DEFINED__

    // {52023973-05fd-46ca-8ac5-bb0aa431fed6}
    MSIX_INTERFACE(IAppxBlockMapFileUtf8,0x52023973,0x05fd,0x46ca,0x8a,0xc5,0xbb,0x0a,0xa4,0x31,0xfe,0xd6);
    interface IAppxBlockMapFileUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPSTR *name) noexcept = 0;
    };
#endif 	/* __IAppxBlockMapFileUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapReaderUtf8_INTERFACE_DEFINED__
#define __IAppxBlockMapReaderUtf8_INTERFACE_DEFINED__

    // {1a02d8b4-477a-487e-9815-317cc09acc22}
    MSIX_INTERFACE(IAppxBlockMapReaderUtf8,0x1a02d8b4,0x477a,0x487e,0x98,0x15,0x31,0x7c,0xc0,0x9a,0xcc,0x22);
    interface IAppxBlockMapReaderUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFile(
            /* [string][in] */  LPCSTR filename,
            /* [retval][out] */  IAppxBlockMapFile **file) noexcept = 0;
    };
#endif 	/* __IAppxBlockMapReaderUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfoUtf8_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfoUtf8_INTERFACE_DEFINED__

    // {f1ae9284-f7ee-431c-9a85-03630ada285b}
    MSIX_INTERFACE(IAppxBundleManifestPackageInfoUtf8,0xf1ae9284,0xf7ee,0x431c,0x9a,0x85,0x03,0x63,0x0a,0xda,0x28,0x5b);
    interface IAppxBundleManifestPackageInfoUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFileName(
            /* [retval][string][out] */  LPSTR *fileName) noexcept = 0;

    };
#endif 	/* __IAppxBundleManifestPackageInfoUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleReaderUtf8_INTERFACE_DEFINED__
#define __IAppxBundleReaderUtf8_INTERFACE_DEFINED__

    // {90f4a4fc-f655-43b6-b033-35a8dfc92cbf}
    MSIX_INTERFACE(IAppxBundleReaderUtf8, 0x90f4a4fc,0xf655,0x43b6,0xb0,0x33,0x35,0xa8,0xdf,0xc9,0x2c,0xbf)
    interface IAppxBundleReaderUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPayloadPackage(
            /* [string][in] */  LPCSTR fileName,
            /* [retval][out] */  IAppxFile **payloadPackage) noexcept = 0;
    };
#endif 	/* __IAppxBundleReaderUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxFactoryUtf8_INTERFACE_DEFINED__
#define __IAppxFactoryUtf8_INTERFACE_DEFINED__

    // {16c3c3f5-6d63-47af-8a68-dcc3aeb0cebb}
    MSIX_INTERFACE(IAppxFactoryUtf8,0x16c3c3f5,0x6d63,0x47af,0x8a,0x68,0xdc,0xc3,0xae,0xb0,0xce,0xbb);
    interface IAppxFactoryUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(
            /* [in] */  IStream *blockMapStream,
            /* [in] */  LPCSTR signatureFileName,
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) noexcept = 0;
    };
#endif  /* __IAppxFactoryUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxFileUtf8_INTERFACE_DEFINED__
#define __IAppxFileUtf8_INTERFACE_DEFINED__

    // {73d14f3f-4c55-4b6d-9311-a02bb3538bb6}
    MSIX_INTERFACE(IAppxFileUtf8,0x73d14f3f,0x4c55,0x4b6d,0x93,0x11,0xa0,0x2b,0xb3,0x53,0x8b,0xb6);
    interface IAppxFileUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetContentType(
            /* [retval][string][out] */ LPSTR *contentType) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPSTR *fileName) noexcept = 0;
    };
#endif 	/* __IAppxFileUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestApplicationUtf8_INTERFACE_DEFINED__
#define __IAppxManifestApplicationUtf8_INTERFACE_DEFINED__

    // {3fb53200-bd86-4971-a8ad-2e18c3c02aae}
    MSIX_INTERFACE(IAppxManifestApplicationUtf8,0x3fb53200,0xbd86,0x4971,0xa8,0xad,0x2e,0x18,0xc3,0xc0,0x2a,0xae);
    interface IAppxManifestApplicationUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCSTR name,
            /* [retval][string][out] */  LPSTR *value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(
            /* [retval][string][out] */  LPSTR *appUserModelId) noexcept = 0;
    };
#endif 	/* __IAppxManifestApplicationUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageDependencyUtf8_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependencyUtf8_INTERFACE_DEFINED__

    // {7ff7ba87-d29e-447a-b2e6-31ddc88b94e9}
    MSIX_INTERFACE(IAppxManifestPackageDependencyUtf8,0x7ff7ba87,0xd29e,0x447a,0xb2,0xe6,0x31,0xdd,0xc8,0x8b,0x94,0xe9);
    interface IAppxManifestPackageDependencyUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPSTR *publisher) noexcept = 0;
    };
#endif 	/* __IAppxManifestPackageDependencyUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageIdUtf8_INTERFACE_DEFINED__
#define __IAppxManifestPackageIdUtf8_INTERFACE_DEFINED__

    // {a1f79b28-37f3-4b58-9100-7af898d11f65}
    MSIX_INTERFACE(IAppxManifestPackageIdUtf8,0xa1f79b28,0x37f3,0x4b58,0x91,0x00,0x7a,0xf8,0x98,0xd1,0x1f,0x65);
    interface IAppxManifestPackageIdUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPSTR *publisher) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResourceId(
            /* [retval][string][out] */ LPSTR *resourceId) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE ComparePublisher(
            /* [string][in] */  LPCSTR other,
            /* [retval][out] */  BOOL *isSame) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFullName(
            /* [retval][string][out] */ LPSTR *packageFullName) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */ LPSTR *packageFamilyName) noexcept = 0;
    };
#endif 	/* __IAppxManifestPackageIdUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPropertiesUtf8_INTERFACE_DEFINED__
#define __IAppxManifestPropertiesUtf8_INTERFACE_DEFINED__

    // {1c3fe108-9115-4623-be85-f0fec99ec516}
    MSIX_INTERFACE(IAppxManifestPropertiesUtf8,0x1c3fe108,0x9115,0x4623,0xbe,0x85,0xf0,0xfe,0xc9,0x9e,0xc5,0x16);
    interface IAppxManifestPropertiesUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBoolValue(
            /* [string][in] */  LPCSTR name,
            /* [retval][out] */  BOOL *value) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCSTR name,
            /* [retval][string][out] */ LPSTR *value) noexcept = 0;
    };
#endif 	/* __IAppxManifestPropertiesUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestQualifiedResourceUtf8_INTERFACE_DEFINED__
#define __IAppxManifestQualifiedResourceUtf8_INTERFACE_DEFINED__

    // {2fac530d-87e4-4dd5-99d3-290bb192363e}
    MSIX_INTERFACE(IAppxManifestQualifiedResourceUtf8,0x2fac530d,0x87e4,0x4dd5,0x99,0xd3,0x29,0x0b,0xb1,0x92,0x36,0x3e);
    interface IAppxManifestQualifiedResourceUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLanguage(
            /* [retval][string][out] */  LPSTR *language) noexcept = 0;
    };
#endif 	/* __IAppxManifestQualifiedResourceUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestCapabilitiesEnumeratorUtf8_INTERFACE_DEFINED__
#define __IAppxManifestCapabilitiesEnumeratorUtf8_INTERFACE_DEFINED__

    // {cc422f8e-a4d9-4f2e-bb49-ac3a5ce2a2f0}
    MSIX_INTERFACE(IAppxManifestCapabilitiesEnumeratorUtf8,0xcc422f8e,0xa4d9,0x4f2e,0xbb,0x49,0xac,0x3a,0x5c,0xe2,0xa2,0xf0);
    interface IAppxManifestCapabilitiesEnumeratorUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPSTR *resource) noexcept = 0;
    };
#endif 	/* __IAppxManifestCapabilitiesEnumeratorUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestResourcesEnumeratorUtf8_INTERFACE_DEFINED__
#define __IAppxManifestResourcesEnumeratorUtf8_INTERFACE_DEFINED__

    // {194cfd4a-55db-4832-b1af-0d6805a4532a}
    MSIX_INTERFACE(IAppxManifestResourcesEnumeratorUtf8,0x194cfd4a,0x55db,0x4832,0xb1,0xaf,0x0d,0x68,0x05,0xa4,0x53,0x2a);
    interface IAppxManifestResourcesEnumeratorUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPSTR *resource) noexcept = 0;
    };
#endif 	/* __IAppxManifestResourcesEnumeratorUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestTargetDeviceFamilyUtf8_INTERFACE_DEFINED__
#define __IAppxManifestTargetDeviceFamilyUtf8_INTERFACE_DEFINED__

    // {70adc4fb-33f5-4ed3-aaaf-e5ad74991669}
    MSIX_INTERFACE(IAppxManifestTargetDeviceFamilyUtf8,0x70adc4fb,0x33f5,0x4ed3,0xaa,0xaf,0xe5,0xad,0x74,0x99,0x16,0x69);
    interface IAppxManifestTargetDeviceFamilyUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPSTR *name) noexcept = 0;
    };
#endif 	/* __IAppxManifestTargetDeviceFamilyUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageReaderUtf8_INTERFACE_DEFINED__
#define __IAppxPackageReaderUtf8_INTERFACE_DEFINED__

    // {842ffe95-cf7a-4707-973a-0a2e9ffc3a85}
    MSIX_INTERFACE(IAppxPackageReaderUtf8,0x842ffe95,0xcf7a,0x4707,0x97,0x3a,0x0a,0x2e,0x9f,0xfc,0x3a,0x85);
    interface IAppxPackageReaderUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPayloadFile(
            /* [string][in] */  LPCSTR fileName,
            /* [retval][out] */  IAppxFile **file) noexcept = 0;
    };
#endif 	/* __IAppxPackageReaderUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageWriterUtf8_INTERFACE_DEFINED__
#define __IAppxPackageWriterUtf8_INTERFACE_DEFINED__

    // {578ee26e-642a-4b03-aeda-8a374ff71b5b}
    MSIX_INTERFACE(IAppxPackageWriterUtf8, 0x578ee26e,0x642a,0x4b03,0xae,0xda,0x8a,0x37,0x4f,0xf7,0x1b,0x5b);
    interface IAppxPackageWriterUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFile( 
            /* [string][in] */ LPCSTR fileName,
            /* [string][in] */ LPCSTR contentType,
            /* [in] */ APPX_COMPRESSION_OPTION compressionOption,
            /* [in] */ IStream* inputStream) noexcept = 0;
    };
#endif 	/* __IAppxPackageWriterUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageWriter3Utf8_INTERFACE_DEFINED__
#define __IAppxPackageWriter3Utf8_INTERFACE_DEFINED__

    typedef struct APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8
    {
        IStream* inputStream;
        LPCSTR fileName;
        LPCSTR contentType;
        APPX_COMPRESSION_OPTION compressionOption;
    } 	APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8;

    // {fc8f7fd6-3a35-49cc-b62f-ea0ee839e25e}
    MSIX_INTERFACE(IAppxPackageWriter3Utf8, 0xfc8f7fd6,0x3a35,0x49cc,0xb6,0x2f,0xea,0x0e,0xe8,0x39,0xe2,0x5e);
    interface IAppxPackageWriter3Utf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFiles( 
            /* [in] */ UINT32 fileCount,
            /* [size_is][in] */ APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles,
            /* [in] */ UINT64 memoryLimit) = 0;
    };
#endif 	/* __IAppxPackageWriter3Utf8_INTERFACE_DEFINED__ */
#ifndef __IAppxManifestOptionalPackageInfoUtf8_INTERFACE_DEFINED__
#define __IAppxManifestOptionalPackageInfoUtf8_INTERFACE_DEFINED__

    // {1c781d5a-90df-4202-a5f3-132bd0a89233}
    MSIX_INTERFACE(IAppxManifestOptionalPackageInfoUtf8, 0x1c781d5a, 0x90df, 0x4202, 0xaf, 0xf3, 0x13, 0x2b, 0xd0, 0xa8, 0x92, 0x33);
    interface IAppxManifestOptionalPackageInfoUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMainPackageName(
            /* [retval][string][out] */  LPSTR *mainPackageName) noexcept = 0;
    };
#endif 	/* __IAppxManifestOptionalPackageInfoUtf8_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestMainPackageDependencyUtf8_INTERFACE_DEFINED__
#define __IAppxManifestMainPackageDependencyUtf8_INTERFACE_DEFINED__

    // {080e7850-8e9c-4162-85e2-97983e493c0f}
    MSIX_INTERFACE(IAppxManifestMainPackageDependencyUtf8, 0x080e7850, 0x8e9c, 0x4162, 0x85, 0xe2, 0x97, 0x98, 0x3e, 0x49, 0x3c, 0x0f);
    interface IAppxManifestMainPackageDependencyUtf8 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */  LPSTR *name) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */  LPSTR *publisher) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */  LPSTR *packageFamilyName) noexcept = 0;
    };
#endif 	/* __IAppxManifestMainPackageDependencyUtf8_INTERFACE_DEFINED__ */

extern "C++" {
typedef /* [v1_enum] */
enum MSIX_VALIDATION_OPTION
    {
        MSIX_VALIDATION_OPTION_FULL                        = 0x0,
        MSIX_VALIDATION_OPTION_SKIPSIGNATURE               = 0x1,
        MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN = 0x2,
        MSIX_VALIDATION_OPTION_SKIPAPPXMANIFEST_DEPRECATED = 0x4, // AppxManifest.xml must be always be valid.
                                                                  // If the SDK is compiled without USE_VALIDATION_PARSER,
                                                                  // no schema validation is done, but it needs to be
                                                                  // valid xml.
    }   MSIX_VALIDATION_OPTION;

typedef /* [v1_enum] */
enum MSIX_PACKUNPACK_OPTION
    {
        MSIX_PACKUNPACK_OPTION_NONE                    = 0x0,
        MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER  = 0x1,
        MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE = 0x2
    }   MSIX_PACKUNPACK_OPTION;

typedef /* [v1_enum] */
enum MSIX_PLATFORMS
    {
        MSIX_PLATFORM_NONE      = 0x0,
        MSIX_PLATFORM_WINDOWS10 = 0x1,
        MSIX_PLATFORM_WINDOWS8  = 0x2,
        MSIX_PLATFORM_WINDOWS7  = 0x4,
        MSIX_PLATFORM_MACOS     = 0x8,
        MSIX_PLATFORM_IOS       = 0x10,
        MSIX_PLATFORM_AOSP      = 0x20,
        MSIX_PLATFORM_LINUX     = 0x40,
        MSIX_PLATFORM_WEB       = 0x80,
        MSIX_PLATFORM_CORE      = 0x100,
    }   MSIX_PLATFORMS;

typedef /* [v1_enum] */
enum MSIX_APPLICABILITY_OPTIONS
    {
        MSIX_APPLICABILITY_OPTION_FULL         = 0x0,
        MSIX_APPLICABILITY_OPTION_SKIPPLATFORM = 0x1,
        MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE = 0x2,
    }   MSIX_APPLICABILITY_OPTIONS;

typedef /* [v1_enum] */
enum MSIX_COMMON_OPTIONS
    {
        MSIX_OPTION_NONE = 0x0,
        MSIX_OPTION_VERBOSE = 0x1,
        MSIX_OPTION_OVERWRITE = 0x2,
        MSIX_OPTION_NOOVERWRITE = 0x4,
        MSIX_OPTION_VERSION = 0x8,
    }   MSIX_COMMON_OPTIONS;

typedef /* [v1_enum] */
enum MSIX_BUNDLE_OPTIONS
    {
        MSIX_BUNDLE_OPTION_NONE = 0x0,
    }   MSIX_BUNDLE_OPTIONS;

#define MSIX_PLATFORM_ALL MSIX_PLATFORM_WINDOWS10      | \
                          MSIX_PLATFORM_WINDOWS10      | \
                          MSIX_PLATFORM_WINDOWS8       | \
                          MSIX_PLATFORM_WINDOWS7       | \
                          MSIX_PLATFORM_MACOS          | \
                          MSIX_PLATFORM_IOS            | \
                          MSIX_PLATFORM_AOSP           | \
                          MSIX_PLATFORM_LINUX          | \
                          MSIX_PLATFORM_WEB            | \
                          MSIX_PLATFORM_CORE             \

#define MSIX_APPLICABILITY_NONE MSIX_APPLICABILITY_OPTION_SKIPPLATFORM         | \
                                MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE           \

// Unpack
MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination
) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackageFromPackageReader(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    IAppxPackageReader* packageReader,
    char* utf8Destination
) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackageFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    IStream* stream,
    char* utf8Destination
) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundle(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    char* utf8SourcePackage,
    char* utf8Destination
) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromBundleReader(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    IAppxBundleReader* bundleReader,
    char* utf8Destination
) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IStream* stream,
    char* utf8Destination
) noexcept;

#ifdef MSIX_PACK

MSIX_API HRESULT STDMETHODCALLTYPE PackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* directoryPath,
    char* outputPackage
) noexcept;

#endif // MSIX_PACK

MSIX_API HRESULT STDMETHODCALLTYPE CreateBundle(
    MSIX_COMMON_OPTIONS commonOptions,
    MSIX_BUNDLE_OPTIONS bundleOptions,
    char* directoryPath,
    char* outputBundle,
    char* mappingFile,
    char* version
) noexcept;

// A call to called CoCreateAppxFactory is required before start using the factory on non-windows platforms specifying
// their allocator/de-allocator pair of preference. Failure to do this will result on E_UNEXPECTED.
typedef LPVOID STDMETHODCALLTYPE COTASKMEMALLOC(SIZE_T cb);
typedef void STDMETHODCALLTYPE COTASKMEMFREE(LPVOID pv);

MSIX_API HRESULT STDMETHODCALLTYPE MsixGetLogTextUTF8(COTASKMEMALLOC* memalloc, char** logText) noexcept;

#ifndef MSIX_DEFINE_GetLogTextUTF8_BACKCOMPAT
#define MSIX_DEFINE_GetLogTextUTF8_BACKCOMPAT 1
#endif

#if MSIX_DEFINE_GetLogTextUTF8_BACKCOMPAT
#ifndef GetLogTextUTF8
#define GetLogTextUTF8(memalloc, logText) MsixGetLogTextUTF8(memalloc, logText)
#endif
#endif

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactory(
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE CoCreateAppxBundleFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IAppxBundleFactory** appxBundleFactory) noexcept;

// provided as a helper for platforms that do not have an implementation of SHCreateStreamOnFileEx
MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream) noexcept;

MSIX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFileUTF16(
    LPCWSTR utf16File,
    bool forRead,
    IStream** stream) noexcept;

} // extern "C++"

#endif //__appxpackaging_hpp__