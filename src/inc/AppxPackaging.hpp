//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

/* File based on the header created by MIDL compiler version 8.01.0622 of AppxPackaging.idl */

// Changes made to the original AppxPackaging.h file:
//  - Remove pragma warning
//  - Remove Windows specific headers
//  - Remove C style interfaces declaration
//  - Remove MIDL_INTERFACE MACRO
//  - Add IUnknown, ISequentialStream and IStream interfaces
//  - Add MSIX specific funcions and helpers (See bottom of file)
//  - See more changes in AppxPackaging_i.cpp

#ifndef __appxpackaging_hpp__
#define __appxpackaging_hpp__

#include "MSIXWindows.hpp"

#ifdef WIN32
#include <AppxPackaging.h>
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

extern "C"{

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

/* interface IUnknown */
/* [unique][uuid][object][local] */
EXTERN_C const IID IID_IUnknown;

    // {00000000-0000-0000-C000-000000000046}
    interface IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ void **ppvObject) = 0;
        virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
        virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
    };
#endif  /* __IUnknown_INTERFACE_DEFINED__ */

#ifndef __ISequentialStream_INTERFACE_DEFINED__
#define __ISequentialStream_INTERFACE_DEFINED__

/* interface ISequentialStream */
/* [unique][uuid][object] */
EXTERN_C const IID IID_ISequentialStream;

    // {0c733a30-2a1c-11ce-ade5-00aa0044773d}
    interface ISequentialStream : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read(
            /* [out] */  void *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG *pcbRead) = 0;

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write(
            /* [in] */ const void *pv,
            /* [in]] */ ULONG cb,
            /* [out_opt] */ ULONG *pcbWritten) = 0;

    };
#endif  /* __ISequentialStream_INTERFACE_DEFINED__ */

#ifndef __IStream_INTERFACE_DEFINED__
#define __IStream_INTERFACE_DEFINED__

/* interface IStream */
/* [unique][uuid][object] */
EXTERN_C const IID IID_IStream;

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
    interface IStream : public ISequentialStream
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek(
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */  ULARGE_INTEGER *plibNewPosition) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE SetSize(
            /* [in] */ ULARGE_INTEGER libNewSize) noexcept = 0;

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo(
            /* [unique][in] */  IStream *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER *pcbRead,
            /* [out] */ ULARGE_INTEGER *pcbWritten) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Commit(
            /* [in] */ DWORD grfCommitFlags) noexcept = 0;

        virtual HRESULT STDMETHODCALLTYPE Revert( void) noexcept = 0;

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

    typedef /* [v1_enum] */
        enum APPX_COMPRESSION_OPTION
    {
        APPX_COMPRESSION_OPTION_NONE = 0,
        APPX_COMPRESSION_OPTION_NORMAL = 1,
        APPX_COMPRESSION_OPTION_MAXIMUM = 2,
        APPX_COMPRESSION_OPTION_FAST = 3,
        APPX_COMPRESSION_OPTION_SUPERFAST = 4
    } 	APPX_COMPRESSION_OPTION;

    typedef /* [v1_enum] */
        enum APPX_FOOTPRINT_FILE_TYPE
    {
        APPX_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY = 3,
        APPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP = 4
    } 	APPX_FOOTPRINT_FILE_TYPE;

    typedef /* [v1_enum] */
        enum APPX_BUNDLE_FOOTPRINT_FILE_TYPE
    {
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_FIRST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_LAST = 2
    } 	APPX_BUNDLE_FOOTPRINT_FILE_TYPE;

    typedef /* [v1_enum] */
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

    typedef /* [v1_enum] */
        enum APPX_PACKAGE_ARCHITECTURE
    {
        APPX_PACKAGE_ARCHITECTURE_X86 = 0,
        APPX_PACKAGE_ARCHITECTURE_ARM = 5,
        APPX_PACKAGE_ARCHITECTURE_X64 = 9,
        APPX_PACKAGE_ARCHITECTURE_NEUTRAL = 11,
        APPX_PACKAGE_ARCHITECTURE_ARM64 = 12
    } 	APPX_PACKAGE_ARCHITECTURE;

    typedef /* [v1_enum] */
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

    typedef /* [v1_enum] */
        enum APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION = 0,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE = 1
    } 	APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE;

    typedef /* [v1_enum] */
        enum DX_FEATURE_LEVEL
    {
        DX_FEATURE_LEVEL_UNSPECIFIED = 0,
        DX_FEATURE_LEVEL_9 = 1,
        DX_FEATURE_LEVEL_10 = 2,
        DX_FEATURE_LEVEL_11 = 3
    } 	DX_FEATURE_LEVEL;

    typedef /* [v1_enum] */
        enum APPX_CAPABILITY_CLASS_TYPE
    {
        APPX_CAPABILITY_CLASS_DEFAULT = 0,
        APPX_CAPABILITY_CLASS_GENERAL = 0x1,
        APPX_CAPABILITY_CLASS_RESTRICTED = 0x2,
        APPX_CAPABILITY_CLASS_WINDOWS = 0x4,
        APPX_CAPABILITY_CLASS_ALL = 0x7,
        APPX_CAPABILITY_CLASS_CUSTOM = 0x8
    } 	APPX_CAPABILITY_CLASS_TYPE;

/* interface IAppxFactory */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFactory;

    // {beb94909-e451-438b-b5a7-d79e767b75d8}
    interface IAppxFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePackageWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */  APPX_PACKAGE_SETTINGS *settings,
            /* [retval][out] */  IAppxPackageWriter **packageWriter) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreatePackageReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxPackageReader **packageReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateManifestReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxManifestReader **manifestReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBlockMapReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(
            /* [in] */  IStream *blockMapStream,
            /* [in] */  LPCWSTR signatureFileName,
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) = 0;

    };
#endif  /* __IAppxFactory_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageReader_INTERFACE_DEFINED__
#define __IAppxPackageReader_INTERFACE_DEFINED__

/* interface IAppxPackageReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageReader;

    // {b5c49650-99bc-481c-9a34-3d53a4106708}
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

/* interface IAppxPackageWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageWriter;

    // {9099e33b-246f-41e4-881a-008eb613f858}
    interface IAppxPackageWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFile(
            /* [string][in] */  LPCWSTR fileName,
            /* [string][in] */  LPCWSTR contentType,
            /* [in] */ APPX_COMPRESSION_OPTION compressionOption,
            /* [in] */  IStream *inputStream) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close(
            /* [in] */  IStream *manifest) = 0;

    };
#endif 	/* __IAppxPackageWriter_INTERFACE_DEFINED__ */

#ifndef __IAppxFile_INTERFACE_DEFINED__
#define __IAppxFile_INTERFACE_DEFINED__

/* interface IAppxFile */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFile;

    // {91df827b-94fd-468f-827b-57f41b2f6f2e}
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

/* interface IAppxFilesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFilesEnumerator;

    // {f007eeaf-9831-411c-9847-917cdc62d1fe}
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

/* interface IAppxBlockMapReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapReader;

    // {5efec991-bca3-42d1-9ec2-e92d609ec22a}
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

/* interface IAppxBlockMapFile */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapFile;

    // {277672ac-4f63-42c1-8abc-beae3600eb59}
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

/* interface IAppxBlockMapFilesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapFilesEnumerator;

    // {02b856a2-4262-4070-bacb-1a8cbbc42305}
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

/* interface IAppxBlockMapBlock */
/* [unique][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapBlock;

    // {75cf3930-3244-4fe0-a8c8-e0bcb270b889}
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

/* interface IAppxBlockMapBlocksEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapBlocksEnumerator;

    // {6b429b5b-36ef-479e-b9eb-0c1482b49e16}
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

/* interface IAppxManifestReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader;

    // {4e1bd148-55a0-4480-a3d1-15544710637c}
    interface IAppxManifestReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetProperties(
            /* [retval][out] */  IAppxManifestProperties **packageProperties) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageDependencies(
            /* [retval][out] */  IAppxManifestPackageDependenciesEnumerator **dependencies) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCapabilities(
            /* [retval][out] */  APPX_CAPABILITIES *capabilities) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResources(
            /* [retval][out] */  IAppxManifestResourcesEnumerator **resources) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDeviceCapabilities(
            /* [retval][out] */  IAppxManifestDeviceCapabilitiesEnumerator **deviceCapabilities) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPrerequisite(
            /* [string][in] */  LPCWSTR name,
            /* [retval][out] */  UINT64 *value) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetApplications(
            /* [retval][out] */  IAppxManifestApplicationsEnumerator **applications) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **manifestStream) = 0;

    };
#endif 	/* __IAppxManifestReader_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader2_INTERFACE_DEFINED__
#define __IAppxManifestReader2_INTERFACE_DEFINED__

/* interface IAppxManifestReader2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader2;

    // {d06f67bc-b31d-4eba-a8af-638e73e77b4d}
    interface IAppxManifestReader2 : public IAppxManifestReader
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetQualifiedResources(
            /* [retval][out] */  IAppxManifestQualifiedResourcesEnumerator **resources) = 0;

    };
#endif 	/* __IAppxManifestReader2_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader3_INTERFACE_DEFINED__
#define __IAppxManifestReader3_INTERFACE_DEFINED__

/* interface IAppxManifestReader3 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader3;

    // {c43825ab-69b7-400a-9709-cc37f5a72d24}
    interface IAppxManifestReader3 : public IAppxManifestReader2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCapabilitiesByCapabilityClass(
            /* [in] */ APPX_CAPABILITY_CLASS_TYPE capabilityClass,
            /* [retval][out] */  IAppxManifestCapabilitiesEnumerator **capabilities) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetTargetDeviceFamilies(
            /* [retval][out] */  IAppxManifestTargetDeviceFamiliesEnumerator **targetDeviceFamilies) = 0;

    };
#endif 	/* __IAppxManifestReader3_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageId_INTERFACE_DEFINED__
#define __IAppxManifestPackageId_INTERFACE_DEFINED__

/* interface IAppxManifestPackageId */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageId;

    // {283ce2d7-7153-4a91-9649-7a0f7240945f}
    interface IAppxManifestPackageId : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetArchitecture(
            /* [retval][out] */  APPX_PACKAGE_ARCHITECTURE *architecture) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPWSTR *publisher) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetVersion(
            /* [retval][out] */  UINT64 *packageVersion) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResourceId(
            /* [retval][string][out] */ LPWSTR *resourceId) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComparePublisher(
            /* [string][in] */  LPCWSTR other,
            /* [retval][out] */  BOOL *isSame) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFullName(
            /* [retval][string][out] */ LPWSTR *packageFullName) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */ LPWSTR *packageFamilyName) = 0;

    };
#endif 	/* __IAppxManifestPackageId_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestProperties_INTERFACE_DEFINED__
#define __IAppxManifestProperties_INTERFACE_DEFINED__

/* interface IAppxManifestProperties */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestProperties;

    // {03faf64d-f26f-4b2c-aaf7-8fe7789b8bca}
    interface IAppxManifestProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBoolValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][out] */  BOOL *value) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][string][out] */ LPWSTR *value) = 0;

    };
#endif 	/* __IAppxManifestProperties_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestTargetDeviceFamiliesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestTargetDeviceFamiliesEnumerator;

    // {36537f36-27a4-4788-88c0-733819575017}
    interface IAppxManifestTargetDeviceFamiliesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestTargetDeviceFamily **targetDeviceFamily) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestTargetDeviceFamiliesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__
#define __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__

/* interface IAppxManifestTargetDeviceFamily */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestTargetDeviceFamily;

    // {9091b09b-c8d5-4f31-8687-a338259faefb}
    interface IAppxManifestTargetDeviceFamily : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMinVersion(
            /* [retval][out] */  UINT64 *minVersion) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMaxVersionTested(
            /* [retval][out] */  UINT64 *maxVersionTested) = 0;

    };
#endif 	/* __IAppxManifestTargetDeviceFamily_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestPackageDependenciesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageDependenciesEnumerator;

    // {b43bbcf9-65a6-42dd-bac0-8c6741e7f5a4}
    interface IAppxManifestPackageDependenciesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestPackageDependency **dependency) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestPackageDependenciesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestResourcesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestResourcesEnumerator;

    // {de4dfbbd-881a-48bb-858c-d6f2baeae6ed}
    interface IAppxManifestResourcesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *resource) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestDeviceCapabilitiesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestDeviceCapabilitiesEnumerator;

    // {30204541-427b-4a1c-bacf-655bf463a540}
    interface IAppxManifestDeviceCapabilitiesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *deviceCapability) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestDeviceCapabilitiesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestCapabilitiesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestCapabilitiesEnumerator;

    // {11d22258-f470-42c1-b291-8361c5437e41}
    interface IAppxManifestCapabilitiesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][string][out] */ LPWSTR *capability) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestCapabilitiesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestApplicationsEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestApplicationsEnumerator;

    // {9eb8a55a-f04b-4d0d-808d-686185d4847a}
    interface IAppxManifestApplicationsEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestApplication **application) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestApplicationsEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestApplication_INTERFACE_DEFINED__
#define __IAppxManifestApplication_INTERFACE_DEFINED__

/* interface IAppxManifestApplication */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestApplication;

    // {5da89bf4-3773-46be-b650-7e744863b7e8}
    interface IAppxManifestApplication : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStringValue(
            /* [string][in] */  LPCWSTR name,
            /* [retval][string][out] */  LPWSTR *value) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(
            /* [retval][string][out] */  LPWSTR *appUserModelId) = 0;

    };
#endif 	/* __IAppxManifestApplication_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestQualifiedResourcesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestQualifiedResourcesEnumerator;

    // {8ef6adfe-3762-4a8f-9373-2fc5d444c8d2}
    interface IAppxManifestQualifiedResourcesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestQualifiedResource **resource) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestQualifiedResourcesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestQualifiedResource_INTERFACE_DEFINED__
#define __IAppxManifestQualifiedResource_INTERFACE_DEFINED__

/* interface IAppxManifestQualifiedResource */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestQualifiedResource;

    // {3b53a497-3c5c-48d1-9ea3-bb7eac8cd7d4}
    interface IAppxManifestQualifiedResource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLanguage(
            /* [retval][string][out] */  LPWSTR *language) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetScale(
            /* [retval][out] */  UINT32 *scale) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDXFeatureLevel(
            /* [retval][out] */  DX_FEATURE_LEVEL *dxFeatureLevel) = 0;

    };
#endif 	/* __IAppxManifestQualifiedResource_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleFactory_INTERFACE_DEFINED__
#define __IAppxBundleFactory_INTERFACE_DEFINED__

/* interface IAppxBundleFactory */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleFactory;

    // {bba65864-965f-4a5f-855f-f074bdbf3a7b}
    interface IAppxBundleFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateBundleWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */ UINT64 bundleVersion,
            /* [retval][out] */  IAppxBundleWriter **bundleWriter) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBundleReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBundleReader **bundleReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateBundleManifestReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxBundleManifestReader **manifestReader) = 0;

    };
#endif 	/* __IAppxBundleFactory_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleWriter_INTERFACE_DEFINED__
#define __IAppxBundleWriter_INTERFACE_DEFINED__

/* interface IAppxBundleWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleWriter;

    // {ec446fe8-bfec-4c64-ab4f-49f038f0c6d2}
    interface IAppxBundleWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadPackage(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *packageStream) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;

    };
#endif 	/* __IAppxBundleWriter_INTERFACE_DEFINED__ */
#ifndef __IAppxBundleReader_INTERFACE_DEFINED__
#define __IAppxBundleReader_INTERFACE_DEFINED__

/* interface IAppxBundleReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleReader;

    // {dd75b8c0-ba76-43b0-ae0f-68656a1dc5c8}
    interface IAppxBundleReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFootprintFile(
            /* [in] */ APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType,
            /* [retval][out] */  IAppxFile **footprintFile) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetBlockMap(
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetManifest(
            /* [retval][out] */  IAppxBundleManifestReader **manifestReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadPackages(
            /* [retval][out] */  IAppxFilesEnumerator **payloadPackages) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadPackage(
            /* [string][in] */  LPCWSTR fileName,
            /* [retval][out] */  IAppxFile **payloadPackage) = 0;

    };
#endif 	/* __IAppxBundleReader_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestReader_INTERFACE_DEFINED__
#define __IAppxBundleManifestReader_INTERFACE_DEFINED__

/* interface IAppxBundleManifestReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestReader;

    // {cf0ebbc1-cc99-4106-91eb-e67462e04fb0}
    interface IAppxBundleManifestReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageInfoItems(
            /* [retval][out] */  IAppxBundleManifestPackageInfoEnumerator **packageInfoItems) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **manifestStream) = 0;

    };
#endif 	/* __IAppxBundleManifestReader_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageDependency_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependency_INTERFACE_DEFINED__

/* interface IAppxManifestPackageDependency */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageDependency;

    // {e4946b59-733e-43f0-a724-3bde4c1285a0}
    interface IAppxManifestPackageDependency : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPWSTR *publisher) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMinVersion(
            /* [retval][out] */  UINT64 *minVersion) = 0;

    };
#endif 	/* __IAppxManifestPackageDependency_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__

/* interface IAppxBundleManifestPackageInfoEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestPackageInfoEnumerator;

    // {f9b856ee-49a6-4e19-b2b0-6a2406d63a32}
    interface IAppxBundleManifestPackageInfoEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBundleManifestPackageInfo **packageInfo) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__

/* interface IAppxBundleManifestPackageInfo */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestPackageInfo;

    // {54cd06c1-268f-40bb-8ed2-757a9ebaec8d}
    interface IAppxBundleManifestPackageInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageType(
            /* [retval][out] */  APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE *packageType) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFileName(
            /* [retval][string][out] */  LPWSTR *fileName) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetOffset(
            /* [retval][out] */  UINT64 *offset) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSize(
            /* [retval][out] */  UINT64 *size) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResources(
            /* [retval][out] */  IAppxManifestQualifiedResourcesEnumerator **resources) = 0;

    };
#endif 	/* __IAppxBundleManifestPackageInfo_INTERFACE_DEFINED__ */

} // extern "C"
#endif // #ifdef WIN32

// MSIX specific

// MSIX public interfaces
interface IMSIXDocumentElement;
interface IMSIXElement;
interface IMSIXElementEnumerator;

#ifndef __IMSIXDocumentElement_INTERFACE_DEFINED__
#define __IMSIXDocumentElement_INTERFACE_DEFINED__

EXTERN_C DECLSPEC_SELECTANY const IID IID_IMSIXDocumentElement;

    // {07f33ab5-cf2c-43bf-a72c-3525313fcff3}
    interface IMSIXDocumentElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentElement(IMSIXElement** documentElement) = 0;
    };

#endif  /* __IMSIXDocumentElement_INTERFACE_DEFINED__ */

#ifndef __IMSIXElement_INTERFACE_DEFINED__
#define __IMSIXElement_INTERFACE_DEFINED__

EXTERN_C DECLSPEC_SELECTANY const IID IID_IMSIXElement;

    // {448d1d3e-186d-485b-9e42-82f0808ff5d6}
    interface IMSIXElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAttributeValue(LPCWSTR name, LPWSTR* value) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetText(LPWSTR* value) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetElements(LPCWSTR name, IMSIXElementEnumerator** elements) = 0;
    };

#endif  /* __IMSIXElement_INTERFACE_DEFINED__ */

#ifndef __IMSIXElementEnumerator_INTERFACE_DEFINED__
#define __IMSIXElementEnumerator_INTERFACE_DEFINED__

EXTERN_C DECLSPEC_SELECTANY const IID IID_IMSIXElementEnumerator;

    // {2173d338-77ce-4427-9b12-d3e8e7f8a732}
    interface IMSIXElementEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(IMSIXElement** element) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) = 0;
        virtual HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) = 0;
    };

#endif  /* __IMSIXElementEnumerator_INTERFACE_DEFINED__ */

extern "C++" {
typedef /* [v1_enum] */
enum MSIX_VALIDATION_OPTION
    {
        MSIX_VALIDATION_OPTION_FULL                        = 0x0,
        MSIX_VALIDATION_OPTION_SKIPSIGNATURE               = 0x1,
        MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN = 0x2,
        MSIX_VALIDATION_OPTION_SKIPAPPXMANIFEST            = 0x4,
    }   MSIX_VALIDATION_OPTION;

typedef /* [v1_enum] */
enum MSIX_PACKUNPACK_OPTION
    {
        MSIX_PACKUNPACK_OPTION_NONE                    = 0x0,
        MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER  = 0x1
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
    }   MSIX_PLATFORMS;

typedef /* [v1_enum] */
enum MSIX_APPLICABILITY_OPTIONS
    {
        MSIX_APPLICABILITY_OPTION_FULL         = 0x0,
        MSIX_APPLICABILITY_OPTION_SKIPPLATFORM = 0x1,
        MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE = 0x2,
    }   MSIX_APPLICABILITY_OPTIONS;

#define MSIX_PLATFORM_ALL MSIX_PLATFORM_WINDOWS10      | \
                          MSIX_PLATFORM_WINDOWS10      | \
                          MSIX_PLATFORM_WINDOWS8       | \
                          MSIX_PLATFORM_WINDOWS7       | \
                          MSIX_PLATFORM_MACOS          | \
                          MSIX_PLATFORM_IOS            | \
                          MSIX_PLATFORM_AOSP           | \
                          MSIX_PLATFORM_LINUX          | \
                          MSIX_PLATFORM_WEB              \

MSIX_API HRESULT STDMETHODCALLTYPE UnpackPackage(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
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

MSIX_API HRESULT STDMETHODCALLTYPE UnpackBundleFromStream(
    MSIX_PACKUNPACK_OPTION packUnpackOptions,
    MSIX_VALIDATION_OPTION validationOption,
    MSIX_APPLICABILITY_OPTIONS applicabilityOptions,
    IStream* stream,
    char* utf8Destination
) noexcept;

// A call to called CoCreateAppxFactory is required before start using the factory on non-windows platforms specifying 
// their allocator/de-allocator pair of preference. Failure to do this will result on E_UNEXPECTED.
typedef LPVOID STDMETHODCALLTYPE COTASKMEMALLOC(SIZE_T cb);
typedef void STDMETHODCALLTYPE COTASKMEMFREE(LPVOID pv);

MSIX_API HRESULT STDMETHODCALLTYPE GetLogTextUTF8(COTASKMEMALLOC* memalloc, char** logText) noexcept;

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

// Helper used for QueryInterface defines
#define SpecializeUuidOfImpl(Type)                    \
    template<>                                        \
    struct UuidOfImpl<Type>                           \
    {                                                 \
        static constexpr const IID& iid = IID_##Type; \
    };                                                \

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
SpecializeUuidOfImpl(IMSIXDocumentElement);
SpecializeUuidOfImpl(IMSIXElement);
SpecializeUuidOfImpl(IMSIXElementEnumerator);

#endif //__appxpackaging_hpp__