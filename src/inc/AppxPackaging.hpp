 /* File based on the header created by MIDL compiler version 8.01.0622 of AppxPackaging.idl */

/* Compiler settings for Source.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data
    VC __declspec() decoration level:
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

// TODO: add details on what parts of the midl generated header were remove, what was added and what is
// and what is requiered if the interfaces change.

/* Forward Declarations */
#ifndef __appxpackaging_hpp__
#define __appxpackaging_hpp__

#include "AppxWindows.hpp"

#ifdef WIN32
#include <AppxPackaging.h>
#else

// Interfaces
interface IUnknown;
interface ISequentialStream;
interface IStream;
interface IAppxFactory;
interface IAppxFactory2;
interface IAppxPackageReader;
interface IAppxPackageReader2;
interface IAppxPackageWriter;
interface IAppxPackageWriter2;
//interface IAppxPackageWriter3;
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
//interface IAppxManifestReader6;
interface IAppxManifestOptionalPackageInfo;
interface IAppxManifestMainPackageDependenciesEnumerator;
interface IAppxManifestMainPackageDependency;
interface IAppxManifestPackageId;
//interface IAppxManifestPackageId2;
interface IAppxManifestProperties;
interface IAppxManifestTargetDeviceFamiliesEnumerator;
interface IAppxManifestTargetDeviceFamily;
interface IAppxManifestPackageDependenciesEnumerator;
interface IAppxManifestPackageDependency;
interface IAppxManifestPackageDependency2;
interface IAppxManifestResourcesEnumerator;
interface IAppxManifestDeviceCapabilitiesEnumerator;
interface IAppxManifestCapabilitiesEnumerator;
interface IAppxManifestApplicationsEnumerator;
interface IAppxManifestApplication;
interface IAppxManifestQualifiedResourcesEnumerator;
interface IAppxManifestQualifiedResource;
interface IAppxBundleFactory;
interface IAppxBundleWriter;
interface IAppxBundleWriter2;
//interface IAppxBundleWriter3;
interface IAppxBundleReader;
interface IAppxBundleManifestReader;
interface IAppxBundleManifestReader2;
interface IAppxBundleManifestPackageInfoEnumerator;
interface IAppxBundleManifestPackageInfo;
//interface IAppxBundleManifestPackageInfo2;
interface IAppxBundleManifestOptionalBundleInfoEnumerator;
interface IAppxBundleManifestOptionalBundleInfo;
interface IAppxContentGroupFilesEnumerator;
interface IAppxContentGroup;
interface IAppxContentGroupsEnumerator;
interface IAppxContentGroupMapReader;
interface IAppxSourceContentGroupMapReader;
interface IAppxContentGroupMapWriter;
interface IAppxEncryptionFactory;
interface IAppxEncryptionFactory2;
interface IAppxEncryptionFactory3;
//interface IAppxEncryptionFactory4;
interface IAppxEncryptedPackageWriter;
//interface IAppxEncryptedPackageWriter2;
interface IAppxEncryptedBundleWriter;
interface IAppxEncryptedBundleWriter2;
//interface IAppxPackageEditor;

extern "C"{

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

/* interface IUnknown */
/* [unique][uuid][object][local] */
EXTERN_C const IID IID_IUnknown;

    MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
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

    MIDL_INTERFACE("0c733a30-2a1c-11ce-ade5-00aa0044773d")
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

    MIDL_INTERFACE("0000000c-0000-0000-C000-000000000046")
    interface IStream : public ISequentialStream
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek(
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */  ULARGE_INTEGER *plibNewPosition) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetSize(
            /* [in] */ ULARGE_INTEGER libNewSize) = 0;

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo(
            /* [unique][in] */  IStream *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER *pcbRead,
            /* [out] */ ULARGE_INTEGER *pcbWritten) = 0;

        virtual HRESULT STDMETHODCALLTYPE Commit(
            /* [in] */ DWORD grfCommitFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE LockRegion(
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;

        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;

        virtual HRESULT STDMETHODCALLTYPE Stat(
            /* [out] */ STATSTG *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
            /* [out] */ IStream **ppstm) = 0;

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

    typedef struct APPX_PACKAGE_WRITER_PAYLOAD_STREAM
    {
        IStream *inputStream;
        LPCWSTR fileName;
        LPCWSTR contentType;
        APPX_COMPRESSION_OPTION compressionOption;
    } 	APPX_PACKAGE_WRITER_PAYLOAD_STREAM;

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

    MIDL_INTERFACE("beb94909-e451-438b-b5a7-d79e767b75d8")
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


#ifndef __IAppxFactory2_INTERFACE_DEFINED__
#define __IAppxFactory2_INTERFACE_DEFINED__

/* interface IAppxFactory2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFactory2;

    MIDL_INTERFACE("f1346df2-c282-4e22-b918-743a929a8d55")
    interface IAppxFactory2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateContentGroupMapReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxContentGroupMapReader **contentGroupMapReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateSourceContentGroupMapReader(
            /* [in] */  IStream *inputStream,
            /* [retval][out] */  IAppxSourceContentGroupMapReader **reader) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateContentGroupMapWriter(
            /* [in] */  IStream *stream,
            /* [retval][out] */  IAppxContentGroupMapWriter **contentGroupMapWriter) = 0;

    };
#endif 	/* __IAppxFactory2_INTERFACE_DEFINED__ */


#ifndef __IAppxPackageReader_INTERFACE_DEFINED__
#define __IAppxPackageReader_INTERFACE_DEFINED__

/* interface IAppxPackageReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageReader;

    MIDL_INTERFACE("b5c49650-99bc-481c-9a34-3d53a4106708")
    interface IAppxPackageReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlockMap(
            /* [retval][out] */  IAppxBlockMapReader **blockMapReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFootprintFile(
            /* [in] */ APPX_FOOTPRINT_FILE_TYPE type,
            /* [retval][out] */  IAppxFile **file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadFile(
            /* [string][in] */  LPCWSTR fileName,
            /* [retval][out] */  IAppxFile **file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPayloadFiles(
            /* [retval][out] */  IAppxFilesEnumerator **filesEnumerator) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetManifest(
            /* [retval][out] */  IAppxManifestReader **manifestReader) = 0;

    };
#endif 	/* __IAppxPackageReader_INTERFACE_DEFINED__ */


#ifndef __IAppxPackageReader2_INTERFACE_DEFINED__
#define __IAppxPackageReader2_INTERFACE_DEFINED__

/* interface IAppxPackageReader2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageReader2;

    MIDL_INTERFACE("37e8d3d5-1aea-4204-9c50-ff715932c249")
    interface IAppxPackageReader2 : public IAppxPackageReader
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetContentGroupMap(
            /* [retval][out] */  IAppxContentGroupMapReader **contentGroupMapReader) = 0;

    };
#endif 	/* __IAppxPackageReader2_INTERFACE_DEFINED__ */

#ifndef __IAppxPackageWriter_INTERFACE_DEFINED__
#define __IAppxPackageWriter_INTERFACE_DEFINED__

/* interface IAppxPackageWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageWriter;
    MIDL_INTERFACE("9099e33b-246f-41e4-881a-008eb613f858")
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

#ifndef __IAppxPackageWriter2_INTERFACE_DEFINED__
#define __IAppxPackageWriter2_INTERFACE_DEFINED__

/* interface IAppxPackageWriter2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxPackageWriter2;

    MIDL_INTERFACE("2cf5c4fd-e54c-4ea5-ba4e-f8c4b105a8c8")
    interface IAppxPackageWriter2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Close(
            /* [in] */  IStream *manifest,
            /* [in] */  IStream *contentGroupMap) = 0;

    };
#endif 	/* __IAppxPackageWriter2_INTERFACE_DEFINED__ */

//#ifndef __IAppxPackageWriter3_INTERFACE_DEFINED__
//#define __IAppxPackageWriter3_INTERFACE_DEFINED__
//
///* interface IAppxPackageWriter3 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxPackageWriter3;
//
//    MIDL_INTERFACE("a83aacd3-41c0-4501-b8a3-74164f50b2fd")
//    interface IAppxPackageWriter3 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE AddPayloadFiles(
//            /* [in] */ UINT32 fileCount,
//            /* [size_is][in] */ APPX_PACKAGE_WRITER_PAYLOAD_STREAM *payloadFiles,
//            /* [in] */ UINT64 memoryLimit) = 0;
//
//    };
//#endif 	/* __IAppxPackageWriter3_INTERFACE_DEFINED__ */

#ifndef __IAppxFile_INTERFACE_DEFINED__
#define __IAppxFile_INTERFACE_DEFINED__

/* interface IAppxFile */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFile;

    MIDL_INTERFACE("91df827b-94fd-468f-827b-57f41b2f6f2e")
    interface IAppxFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCompressionOption(
            /* [retval][out] */  APPX_COMPRESSION_OPTION *compressionOption) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetContentType(
            /* [retval][string][out] */ LPWSTR *contentType) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *fileName) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSize(
            /* [retval][out] */  UINT64 *size) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **stream) = 0;

    };
#endif 	/* __IAppxFile_INTERFACE_DEFINED__ */

#ifndef __IAppxFilesEnumerator_INTERFACE_DEFINED__
#define __IAppxFilesEnumerator_INTERFACE_DEFINED__

/* interface IAppxFilesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxFilesEnumerator;

    MIDL_INTERFACE("f007eeaf-9831-411c-9847-917cdc62d1fe")
    interface IAppxFilesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxFile **file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxFilesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapReader_INTERFACE_DEFINED__
#define __IAppxBlockMapReader_INTERFACE_DEFINED__

/* interface IAppxBlockMapReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapReader;

    MIDL_INTERFACE("5efec991-bca3-42d1-9ec2-e92d609ec22a")
    interface IAppxBlockMapReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFile(
            /* [string][in] */  LPCWSTR filename,
            /* [retval][out] */  IAppxBlockMapFile **file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFiles(
            /* [retval][out] */  IAppxBlockMapFilesEnumerator **enumerator) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHashMethod(
            /* [retval][out] */  IUri **hashMethod) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetStream(
            /* [retval][out] */  IStream **blockMapStream) = 0;

    };
#endif 	/* __IAppxBlockMapReader_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapFile_INTERFACE_DEFINED__
#define __IAppxBlockMapFile_INTERFACE_DEFINED__

/* interface IAppxBlockMapFile */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapFile;

    MIDL_INTERFACE("277672ac-4f63-42c1-8abc-beae3600eb59")
    interface IAppxBlockMapFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlocks(
            /* [retval][out] */  IAppxBlockMapBlocksEnumerator **blocks) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize(
            /* [retval][out] */  UINT32 *lfhSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetUncompressedSize(
            /* [retval][out] */  UINT64 *size) = 0;

        virtual HRESULT STDMETHODCALLTYPE ValidateFileHash(
            /* [in] */  IStream *fileStream,
            /* [retval][out] */  BOOL *isValid) = 0;

    };
#endif 	/* __IAppxBlockMapFile_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__
#define __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__

/* interface IAppxBlockMapFilesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapFilesEnumerator;

    MIDL_INTERFACE("02b856a2-4262-4070-bacb-1a8cbbc42305")
    interface IAppxBlockMapFilesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBlockMapFile **file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

    };
#endif 	/* __IAppxBlockMapFilesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapBlock_INTERFACE_DEFINED__
#define __IAppxBlockMapBlock_INTERFACE_DEFINED__

/* interface IAppxBlockMapBlock */
/* [unique][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapBlock;

    MIDL_INTERFACE("75cf3930-3244-4fe0-a8c8-e0bcb270b889")
    interface IAppxBlockMapBlock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHash(
            /* [out] */  UINT32 *bufferSize,
            /* [retval][size_is][size_is][out] */  BYTE **buffer) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCompressedSize(
            /* [retval][out] */  UINT32 *size) = 0;

    };
#endif 	/* __IAppxBlockMapBlock_INTERFACE_DEFINED__ */

#ifndef __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__
#define __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__

/* interface IAppxBlockMapBlocksEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBlockMapBlocksEnumerator;

    MIDL_INTERFACE("6b429b5b-36ef-479e-b9eb-0c1482b49e16")
    interface IAppxBlockMapBlocksEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBlockMapBlock **block) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxBlockMapBlocksEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader_INTERFACE_DEFINED__
#define __IAppxManifestReader_INTERFACE_DEFINED__

/* interface IAppxManifestReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader;

    MIDL_INTERFACE("4e1bd148-55a0-4480-a3d1-15544710637c")
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

    MIDL_INTERFACE("d06f67bc-b31d-4eba-a8af-638e73e77b4d")
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

    MIDL_INTERFACE("C43825AB-69B7-400A-9709-CC37F5A72D24")
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

#ifndef __IAppxManifestReader4_INTERFACE_DEFINED__
#define __IAppxManifestReader4_INTERFACE_DEFINED__

/* interface IAppxManifestReader4 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader4;

    MIDL_INTERFACE("4579BB7C-741D-4161-B5A1-47BD3B78AD9B")
    interface IAppxManifestReader4 : public IAppxManifestReader3
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOptionalPackageInfo(
            /* [retval][out] */  IAppxManifestOptionalPackageInfo **optionalPackageInfo) = 0;

    };
#endif 	/* __IAppxManifestReader4_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestReader5_INTERFACE_DEFINED__
#define __IAppxManifestReader5_INTERFACE_DEFINED__

/* interface IAppxManifestReader5 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestReader5;

    MIDL_INTERFACE("8D7AE132-A690-4C00-B75A-6AAE1FEAAC80")
    interface IAppxManifestReader5 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMainPackageDependencies(
            /* [retval][out] */  IAppxManifestMainPackageDependenciesEnumerator **mainPackageDependencies) = 0;

    };
#endif 	/* __IAppxManifestReader5_INTERFACE_DEFINED__ */

//#ifndef __IAppxManifestReader6_INTERFACE_DEFINED__
//#define __IAppxManifestReader6_INTERFACE_DEFINED__
//
///* interface IAppxManifestReader6 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxManifestReader6;
//
//    MIDL_INTERFACE("34DEACA4-D3C0-4E3E-B312-E42625E3807E")
//    interface IAppxManifestReader6 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE GetIsNonQualifiedResourcePackage(
//            /* [retval][out] */  BOOL *isNonQualifiedResourcePackage) = 0;
//
//    };
//#endif 	/* __IAppxManifestReader6_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__
#define __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__

/* interface IAppxManifestOptionalPackageInfo */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestOptionalPackageInfo;

    MIDL_INTERFACE("2634847D-5B5D-4FE5-A243-002FF95EDC7E")
    interface IAppxManifestOptionalPackageInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIsOptionalPackage(
            /* [retval][out] */  BOOL *isOptionalPackage) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetMainPackageName(
            /* [retval][string][out] */ LPWSTR *mainPackageName) = 0;

    };
#endif 	/* __IAppxManifestOptionalPackageInfo_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestMainPackageDependenciesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestMainPackageDependenciesEnumerator;

    MIDL_INTERFACE("A99C4F00-51D2-4F0F-BA46-7ED5255EBDFF")
    interface IAppxManifestMainPackageDependenciesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxManifestMainPackageDependency **mainPackageDependency) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxManifestMainPackageDependenciesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__
#define __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__

/* interface IAppxManifestMainPackageDependency */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestMainPackageDependency;

    MIDL_INTERFACE("05D0611C-BC29-46D5-97E2-84B9C79BD8AE")
    interface IAppxManifestMainPackageDependency : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */ LPWSTR *name) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublisher(
            /* [retval][string][out] */ LPWSTR *publisher) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
            /* [retval][string][out] */ LPWSTR *packageFamilyName) = 0;

    };
#endif 	/* __IAppxManifestMainPackageDependency_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestPackageId_INTERFACE_DEFINED__
#define __IAppxManifestPackageId_INTERFACE_DEFINED__

/* interface IAppxManifestPackageId */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageId;

    MIDL_INTERFACE("283ce2d7-7153-4a91-9649-7a0f7240945f")
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


//#ifndef __IAppxManifestPackageId2_INTERFACE_DEFINED__
//#define __IAppxManifestPackageId2_INTERFACE_DEFINED__
//
///* interface IAppxManifestPackageId2 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxManifestPackageId2;
//
//    MIDL_INTERFACE("2256999d-d617-42f1-880e-0ba4542319d5")
//    interface IAppxManifestPackageId2 : public IAppxManifestPackageId
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE GetArchitecture2(
//            /* [retval][out] */  APPX_PACKAGE_ARCHITECTURE2 *architecture) = 0;
//
//    };
//#endif 	/* __IAppxManifestPackageId2_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestProperties_INTERFACE_DEFINED__
#define __IAppxManifestProperties_INTERFACE_DEFINED__

/* interface IAppxManifestProperties */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestProperties;

    MIDL_INTERFACE("03faf64d-f26f-4b2c-aaf7-8fe7789b8bca")
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

    MIDL_INTERFACE("36537F36-27A4-4788-88C0-733819575017")
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

    MIDL_INTERFACE("9091B09B-C8D5-4F31-8687-A338259FAEFB")
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

    MIDL_INTERFACE("b43bbcf9-65a6-42dd-bac0-8c6741e7f5a4")
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

#ifndef __IAppxManifestPackageDependency_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependency_INTERFACE_DEFINED__

/* interface IAppxManifestPackageDependency */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageDependency;

    MIDL_INTERFACE("e4946b59-733e-43f0-a724-3bde4c1285a0")
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

#ifndef __IAppxManifestPackageDependency2_INTERFACE_DEFINED__
#define __IAppxManifestPackageDependency2_INTERFACE_DEFINED__

/* interface IAppxManifestPackageDependency2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestPackageDependency2;

    MIDL_INTERFACE("DDA0B713-F3FF-49D3-898A-2786780C5D98")
    interface IAppxManifestPackageDependency2 : public IAppxManifestPackageDependency
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMaxMajorVersionTested(
            /* [retval][out] */  UINT16 *maxMajorVersionTested) = 0;

    };
#endif 	/* __IAppxManifestPackageDependency2_INTERFACE_DEFINED__ */

#ifndef __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__
#define __IAppxManifestResourcesEnumerator_INTERFACE_DEFINED__

/* interface IAppxManifestResourcesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxManifestResourcesEnumerator;

    MIDL_INTERFACE("de4dfbbd-881a-48bb-858c-d6f2baeae6ed")
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

    MIDL_INTERFACE("30204541-427b-4a1c-bacf-655bf463a540")
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

    MIDL_INTERFACE("11D22258-F470-42C1-B291-8361C5437E41")
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

    MIDL_INTERFACE("9eb8a55a-f04b-4d0d-808d-686185d4847a")
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

    MIDL_INTERFACE("5da89bf4-3773-46be-b650-7e744863b7e8")
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

    MIDL_INTERFACE("8ef6adfe-3762-4a8f-9373-2fc5d444c8d2")
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

    MIDL_INTERFACE("3b53a497-3c5c-48d1-9ea3-bb7eac8cd7d4")
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

    MIDL_INTERFACE("BBA65864-965F-4A5F-855F-F074BDBF3A7B")
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

    MIDL_INTERFACE("EC446FE8-BFEC-4C64-AB4F-49F038F0C6D2")
    interface IAppxBundleWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadPackage(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *packageStream) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;

    };
#endif 	/* __IAppxBundleWriter_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleWriter2_INTERFACE_DEFINED__
#define __IAppxBundleWriter2_INTERFACE_DEFINED__

/* interface IAppxBundleWriter2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleWriter2;

    MIDL_INTERFACE("6D8FE971-01CC-49A0-B685-233851279962")
    interface IAppxBundleWriter2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddExternalPackageReference(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *inputStream) = 0;

    };
#endif 	/* __IAppxBundleWriter2_INTERFACE_DEFINED__ */

//#ifndef __IAppxBundleWriter3_INTERFACE_DEFINED__
//#define __IAppxBundleWriter3_INTERFACE_DEFINED__
//
///* interface IAppxBundleWriter3 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxBundleWriter3;
//
//    MIDL_INTERFACE("AD711152-F969-4193-82D5-9DDF2786D21A")
//    interface IAppxBundleWriter3 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE AddPackageReference(
//            /* [string][in] */  LPCWSTR fileName,
//            /* [in] */  IStream *inputStream) = 0;
//
//        virtual HRESULT STDMETHODCALLTYPE Close(
//            /* [string][in] */  LPCWSTR hashMethodString) = 0;
//
//    };
//#endif 	/* __IAppxBundleWriter3_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleReader_INTERFACE_DEFINED__
#define __IAppxBundleReader_INTERFACE_DEFINED__

/* interface IAppxBundleReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleReader;

    MIDL_INTERFACE("DD75B8C0-BA76-43B0-AE0F-68656A1DC5C8")
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

    MIDL_INTERFACE("CF0EBBC1-CC99-4106-91EB-E67462E04FB0")
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

#ifndef __IAppxBundleManifestReader2_INTERFACE_DEFINED__
#define __IAppxBundleManifestReader2_INTERFACE_DEFINED__

/* interface IAppxBundleManifestReader2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestReader2;

    MIDL_INTERFACE("5517DF70-033F-4AF2-8213-87D766805C02")
    interface IAppxBundleManifestReader2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOptionalBundles(
            /* [retval][out] */  IAppxBundleManifestOptionalBundleInfoEnumerator **optionalBundles) = 0;

    };
#endif 	/* __IAppxBundleManifestReader2_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__
#define __IAppxBundleManifestPackageInfoEnumerator_INTERFACE_DEFINED__

/* interface IAppxBundleManifestPackageInfoEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestPackageInfoEnumerator;

    MIDL_INTERFACE("F9B856EE-49A6-4E19-B2B0-6A2406D63A32")
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

    MIDL_INTERFACE("54CD06C1-268F-40BB-8ED2-757A9EBAEC8D")
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

//#ifndef __IAppxBundleManifestPackageInfo2_INTERFACE_DEFINED__
//#define __IAppxBundleManifestPackageInfo2_INTERFACE_DEFINED__
//
///* interface IAppxBundleManifestPackageInfo2 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxBundleManifestPackageInfo2;
//
//    MIDL_INTERFACE("44C2ACBC-B2CF-4CCB-BBDB-9C6DA8C3BC9E")
//    interface IAppxBundleManifestPackageInfo2 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE GetIsPackageReference(
//            /* [retval][out] */  BOOL *isPackageReference) = 0;
//
//        virtual HRESULT STDMETHODCALLTYPE GetIsNonQualifiedResourcePackage(
//            /* [retval][out] */  BOOL *isNonQualifiedResourcePackage) = 0;
//
//        virtual HRESULT STDMETHODCALLTYPE GetIsDefaultApplicablePackage(
//            /* [retval][out] */  BOOL *isDefaultApplicablePackage) = 0;
//
//    };
//#endif 	/* __IAppxBundleManifestPackageInfo2_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestOptionalBundleInfoEnumerator_INTERFACE_DEFINED__
#define __IAppxBundleManifestOptionalBundleInfoEnumerator_INTERFACE_DEFINED__

/* interface IAppxBundleManifestOptionalBundleInfoEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestOptionalBundleInfoEnumerator;

    MIDL_INTERFACE("9A178793-F97E-46AC-AACA-DD5BA4C177C8")
    interface IAppxBundleManifestOptionalBundleInfoEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxBundleManifestOptionalBundleInfo **optionalBundle) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxBundleManifestOptionalBundleInfoEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxBundleManifestOptionalBundleInfo_INTERFACE_DEFINED__
#define __IAppxBundleManifestOptionalBundleInfo_INTERFACE_DEFINED__

/* interface IAppxBundleManifestOptionalBundleInfo */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxBundleManifestOptionalBundleInfo;

    MIDL_INTERFACE("515BF2E8-BCB0-4D69-8C48-E383147B6E12")
    interface IAppxBundleManifestOptionalBundleInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPackageId(
            /* [retval][out] */  IAppxManifestPackageId **packageId) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFileName(
            /* [retval][string][out] */  LPWSTR *fileName) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPackageInfoItems(
            /* [retval][out] */  IAppxBundleManifestPackageInfoEnumerator **packageInfoItems) = 0;

    };
#endif 	/* __IAppxBundleManifestOptionalBundleInfo_INTERFACE_DEFINED__ */

#ifndef __IAppxContentGroupFilesEnumerator_INTERFACE_DEFINED__
#define __IAppxContentGroupFilesEnumerator_INTERFACE_DEFINED__

/* interface IAppxContentGroupFilesEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxContentGroupFilesEnumerator;

    MIDL_INTERFACE("1a09a2fd-7440-44eb-8c84-848205a6a1cc")
    interface IAppxContentGroupFilesEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  LPWSTR *file) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxContentGroupFilesEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxContentGroup_INTERFACE_DEFINED__
#define __IAppxContentGroup_INTERFACE_DEFINED__

/* interface IAppxContentGroup */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxContentGroup;

    MIDL_INTERFACE("328f6468-c04f-4e3c-b6fa-6b8d27f3003a")
    interface IAppxContentGroup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName(
            /* [retval][string][out] */  LPWSTR *groupName) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFiles(
            /* [retval][out] */  IAppxContentGroupFilesEnumerator **enumerator) = 0;

    };
#endif 	/* __IAppxContentGroup_INTERFACE_DEFINED__ */

#ifndef __IAppxContentGroupsEnumerator_INTERFACE_DEFINED__
#define __IAppxContentGroupsEnumerator_INTERFACE_DEFINED__

/* interface IAppxContentGroupsEnumerator */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxContentGroupsEnumerator;

    MIDL_INTERFACE("3264e477-16d1-4d63-823e-7d2984696634")
    interface IAppxContentGroupsEnumerator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(
            /* [retval][out] */  IAppxContentGroup **stream) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
            /* [retval][out] */  BOOL *hasCurrent) = 0;

        virtual HRESULT STDMETHODCALLTYPE MoveNext(
            /* [retval][out] */  BOOL *hasNext) = 0;

    };
#endif 	/* __IAppxContentGroupsEnumerator_INTERFACE_DEFINED__ */

#ifndef __IAppxContentGroupMapReader_INTERFACE_DEFINED__
#define __IAppxContentGroupMapReader_INTERFACE_DEFINED__

/* interface IAppxContentGroupMapReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxContentGroupMapReader;

    MIDL_INTERFACE("418726d8-dd99-4f5d-9886-157add20de01")
    interface IAppxContentGroupMapReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRequiredGroup(
            /* [retval][out] */  IAppxContentGroup **requiredGroup) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAutomaticGroups(
            /* [retval][out] */  IAppxContentGroupsEnumerator **automaticGroupsEnumerator) = 0;

    };
#endif 	/* __IAppxContentGroupMapReader_INTERFACE_DEFINED__ */

#ifndef __IAppxSourceContentGroupMapReader_INTERFACE_DEFINED__
#define __IAppxSourceContentGroupMapReader_INTERFACE_DEFINED__

/* interface IAppxSourceContentGroupMapReader */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxSourceContentGroupMapReader;

    MIDL_INTERFACE("f329791d-540b-4a9f-bc75-3282b7d73193")
    interface IAppxSourceContentGroupMapReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRequiredGroup(
            /* [retval][out] */  IAppxContentGroup **requiredGroup) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetAutomaticGroups(
            /* [retval][out] */  IAppxContentGroupsEnumerator **automaticGroupsEnumerator) = 0;

    };
#endif 	/* __IAppxSourceContentGroupMapReader_INTERFACE_DEFINED__ */

#ifndef __IAppxContentGroupMapWriter_INTERFACE_DEFINED__
#define __IAppxContentGroupMapWriter_INTERFACE_DEFINED__

/* interface IAppxContentGroupMapWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxContentGroupMapWriter;

    MIDL_INTERFACE("d07ab776-a9de-4798-8c14-3db31e687c78")
    interface IAppxContentGroupMapWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddAutomaticGroup(
            /* [string][in] */  LPCWSTR groupName) = 0;

        virtual HRESULT STDMETHODCALLTYPE AddAutomaticFile(
            /* [string][in] */  LPCWSTR fileName) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;

    };
#endif 	/* __IAppxContentGroupMapWriter_INTERFACE_DEFINED__ */

typedef struct APPX_ENCRYPTED_PACKAGE_SETTINGS
    {
    UINT32 keyLength;
    LPCWSTR encryptionAlgorithm;
    BOOL useDiffusion;
    IUri *blockMapHashAlgorithm;
    } 	APPX_ENCRYPTED_PACKAGE_SETTINGS;

typedef /* [v1_enum] */
enum APPX_ENCRYPTED_PACKAGE_OPTIONS
    {
        APPX_ENCRYPTED_PACKAGE_OPTION_NONE	= 0,
        APPX_ENCRYPTED_PACKAGE_OPTION_DIFFUSION	= 0x1,
        APPX_ENCRYPTED_PACKAGE_OPTION_PAGE_HASHING	= 0x2
    } 	APPX_ENCRYPTED_PACKAGE_OPTIONS;

typedef struct APPX_ENCRYPTED_PACKAGE_SETTINGS2
    {
    UINT32 keyLength;
    LPCWSTR encryptionAlgorithm;
    IUri *blockMapHashAlgorithm;
    DWORD options;
    } 	APPX_ENCRYPTED_PACKAGE_SETTINGS2;

typedef struct APPX_KEY_INFO
    {
    UINT32 keyLength;
    UINT32 keyIdLength;
    /* [size_is] */ BYTE *key;
    /* [size_is] */ BYTE *keyId;
    } 	APPX_KEY_INFO;

typedef struct APPX_ENCRYPTED_EXEMPTIONS
    {
    UINT32 count;
    /* [unique][size_is] */ LPCWSTR *plainTextFiles;
    } 	APPX_ENCRYPTED_EXEMPTIONS;

#ifndef __IAppxEncryptionFactory_INTERFACE_DEFINED__
#define __IAppxEncryptionFactory_INTERFACE_DEFINED__

/* interface IAppxEncryptionFactory */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptionFactory;

    MIDL_INTERFACE("80E8E04D-8C88-44AE-A011-7CADF6FB2E72")
    interface IAppxEncryptionFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EncryptPackage(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles) = 0;

        virtual HRESULT STDMETHODCALLTYPE DecryptPackage(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_KEY_INFO *keyInfo) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedPackageWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */  IStream *manifestStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
            /* [retval][out] */  IAppxEncryptedPackageWriter **packageWriter) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedPackageReader(
            /* [in] */  IStream *inputStream,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [retval][out] */  IAppxPackageReader **packageReader) = 0;

        virtual HRESULT STDMETHODCALLTYPE EncryptBundle(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles) = 0;

        virtual HRESULT STDMETHODCALLTYPE DecryptBundle(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_KEY_INFO *keyInfo) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedBundleWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */ UINT64 bundleVersion,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
            /* [retval][out] */  IAppxEncryptedBundleWriter **bundleWriter) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedBundleReader(
            /* [in] */  IStream *inputStream,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [retval][out] */  IAppxBundleReader **bundleReader) = 0;

    };
#endif 	/* __IAppxEncryptionFactory_INTERFACE_DEFINED__ */

#ifndef __IAppxEncryptionFactory2_INTERFACE_DEFINED__
#define __IAppxEncryptionFactory2_INTERFACE_DEFINED__

/* interface IAppxEncryptionFactory2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptionFactory2;

    MIDL_INTERFACE("c1b11eee-c4ba-4ab2-a55d-d015fe8ff64f")
    interface IAppxEncryptionFactory2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedPackageWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */  IStream *manifestStream,
            /* [in] */  IStream *contentGroupMapStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
            /* [retval][out] */  IAppxEncryptedPackageWriter **packageWriter) = 0;

    };
#endif 	/* __IAppxEncryptionFactory2_INTERFACE_DEFINED__ */

#ifndef __IAppxEncryptionFactory3_INTERFACE_DEFINED__
#define __IAppxEncryptionFactory3_INTERFACE_DEFINED__

/* interface IAppxEncryptionFactory3 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptionFactory3;

    MIDL_INTERFACE("09edca37-cd64-47d6-b7e8-1cb11d4f7e05")
    interface IAppxEncryptionFactory3 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EncryptPackage(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS2 *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedPackageWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */  IStream *manifestStream,
            /* [in] */  IStream *contentGroupMapStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS2 *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
            /* [retval][out] */  IAppxEncryptedPackageWriter **packageWriter) = 0;

        virtual HRESULT STDMETHODCALLTYPE EncryptBundle(
            /* [in] */  IStream *inputStream,
            /* [in] */  IStream *outputStream,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS2 *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateEncryptedBundleWriter(
            /* [in] */  IStream *outputStream,
            /* [in] */ UINT64 bundleVersion,
            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS2 *settings,
            /* [in] */  const APPX_KEY_INFO *keyInfo,
            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
            /* [retval][out] */  IAppxEncryptedBundleWriter **bundleWriter) = 0;

    };
#endif 	/* __IAppxEncryptionFactory3_INTERFACE_DEFINED__ */

//#ifndef __IAppxEncryptionFactory4_INTERFACE_DEFINED__
//#define __IAppxEncryptionFactory4_INTERFACE_DEFINED__
//
///* interface IAppxEncryptionFactory4 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxEncryptionFactory4;
//
//    MIDL_INTERFACE("A879611F-12FD-41fe-85D5-06AE779BBAF5")
//    interface IAppxEncryptionFactory4 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE EncryptPackage(
//            /* [in] */  IStream *inputStream,
//            /* [in] */  IStream *outputStream,
//            /* [in] */  const APPX_ENCRYPTED_PACKAGE_SETTINGS2 *settings,
//            /* [in] */  const APPX_KEY_INFO *keyInfo,
//            /* [in] */  const APPX_ENCRYPTED_EXEMPTIONS *exemptedFiles,
//            /* [in] */ UINT64 memoryLimit) = 0;
//
//    };
//#endif 	/* __IAppxEncryptionFactory4_INTERFACE_DEFINED__ */

#ifndef __IAppxEncryptedPackageWriter_INTERFACE_DEFINED__
#define __IAppxEncryptedPackageWriter_INTERFACE_DEFINED__

/* interface IAppxEncryptedPackageWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptedPackageWriter;

    MIDL_INTERFACE("F43D0B0B-1379-40E2-9B29-682EA2BF42AF")
    interface IAppxEncryptedPackageWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadFileEncrypted(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */ APPX_COMPRESSION_OPTION compressionOption,
            /* [in] */  IStream *inputStream) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;

    };
#endif 	/* __IAppxEncryptedPackageWriter_INTERFACE_DEFINED__ */

//#ifndef __IAppxEncryptedPackageWriter2_INTERFACE_DEFINED__
//#define __IAppxEncryptedPackageWriter2_INTERFACE_DEFINED__
//
///* interface IAppxEncryptedPackageWriter2 */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxEncryptedPackageWriter2;
//
//    MIDL_INTERFACE("3E475447-3A25-40b5-8AD2-F953AE50C92D")
//    interface IAppxEncryptedPackageWriter2 : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE AddPayloadFilesEncrypted(
//            /* [in] */ UINT32 fileCount,
//            /* [size_is][in] */  APPX_PACKAGE_WRITER_PAYLOAD_STREAM *payloadFiles,
//            /* [in] */ UINT64 memoryLimit) = 0;
//
//    };
//#endif 	/* __IAppxEncryptedPackageWriter2_INTERFACE_DEFINED__ */

#ifndef __IAppxEncryptedBundleWriter_INTERFACE_DEFINED__
#define __IAppxEncryptedBundleWriter_INTERFACE_DEFINED__

/* interface IAppxEncryptedBundleWriter */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptedBundleWriter;

    MIDL_INTERFACE("80B0902F-7BF0-4117-B8C6-4279EF81EE77")
    interface IAppxEncryptedBundleWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPayloadPackageEncrypted(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *packageStream) = 0;

        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;

    };
#endif 	/* __IAppxEncryptedBundleWriter_INTERFACE_DEFINED__ */

#ifndef __IAppxEncryptedBundleWriter2_INTERFACE_DEFINED__
#define __IAppxEncryptedBundleWriter2_INTERFACE_DEFINED__

/* interface IAppxEncryptedBundleWriter2 */
/* [ref][uuid][object] */
EXTERN_C const IID IID_IAppxEncryptedBundleWriter2;

    MIDL_INTERFACE("E644BE82-F0FA-42B8-A956-8D1CB48EE379")
    interface IAppxEncryptedBundleWriter2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddExternalPackageReference(
            /* [string][in] */  LPCWSTR fileName,
            /* [in] */  IStream *inputStream) = 0;

    };
#endif 	/* __IAppxEncryptedBundleWriter2_INTERFACE_DEFINED__ */

//#ifndef __IAppxPackageEditor_INTERFACE_DEFINED__
//#define __IAppxPackageEditor_INTERFACE_DEFINED__
//
///* interface IAppxPackageEditor */
///* [ref][uuid][object] */
//EXTERN_C const IID IID_IAppxPackageEditor;
//
//    MIDL_INTERFACE("E2ADB6DC-5E71-4416-86B6-86E5F5291A6B")
//    interface IAppxPackageEditor : public IUnknown
//    {
//    public:
//        virtual HRESULT STDMETHODCALLTYPE CreateDeltaPackage(
//            /* [in] */  IStream *updatedPackageStream,
//            /* [in] */  IStream *baselinePackageStream,
//            /* [in] */  IStream *deltaPackageStream,
//            /* [string][in] */  LPCWSTR workingDirectory) = 0;
//
//        virtual HRESULT STDMETHODCALLTYPE CreateDeltaPackageUsingBaselineBlockMap(
//            /* [in] */  IStream *updatedPackageStream,
//            /* [in] */  IStream *baselineBlockMapStream,
//            /* [string][in] */  LPCWSTR baselinePackageFullName,
//            /* [in] */  IStream *deltaPackageStream,
//            /* [string][in] */  LPCWSTR workingDirectory) = 0;
//
//    };
//#endif 	/* __IAppxPackageEditor_INTERFACE_DEFINED__ */

}
#endif

extern "C++" {
typedef /* [v1_enum] */
enum APPX_VALIDATION_OPTION
    {
        APPX_VALIDATION_OPTION_FULL                        = 0x0,
        APPX_VALIDATION_OPTION_SKIPSIGNATURE               = 0x1,
        APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN = 0x2,
        APPX_VALIDATION_OPTION_SKIPAPPXMANIFEST            = 0x4
    }   APPX_VALIDATION_OPTION;

typedef /* [v1_enum] */
enum APPX_PACKUNPACK_OPTION
    {
        APPX_PACKUNPACK_OPTION_NONE                    = 0x0,
        APPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER  = 0x1
    }   APPX_PACKUNPACK_OPTION;

XPLATAPPX_API HRESULT STDMETHODCALLTYPE PackAppx(
    APPX_PACKUNPACK_OPTION packUnpackOptions,
    APPX_VALIDATION_OPTION validationOption,
    char* utf8FolderToPack,
    char* utf8CertificatePath,
    char* utf8Destination
);

XPLATAPPX_API HRESULT STDMETHODCALLTYPE UnpackAppx(
    APPX_PACKUNPACK_OPTION packUnpackOptions,
    APPX_VALIDATION_OPTION validationOption,
    char* utf8SourcePackage,
    char* utf8Destination
);

// TODO: this export is temporary, remove later.
XPLATAPPX_API HRESULT STDMETHODCALLTYPE ValidateAppxSignature(char* appx);

// A call to called CoCreateAppxFactory is required before start using the factory on non-windows platforms specifying 
// their allocator/de-allocator pair of preference. Failure to do this will result on E_UNEXPECTED.
typedef LPVOID STDMETHODCALLTYPE COTASKMEMALLOC(SIZE_T cb);
typedef void STDMETHODCALLTYPE COTASKMEMFREE(LPVOID pv);

// Call specific for Windows. Default to call CoTaskMemAlloc and CoTaskMemFree
XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactory(
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory);
}

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CoCreateAppxFactoryWithHeap(
    COTASKMEMALLOC* memalloc,
    COTASKMEMFREE* memfree,
    APPX_VALIDATION_OPTION validationOption,
    IAppxFactory** appxFactory);

// provided as a helper for platforms that do not have an implementation of SHCreateStreamOnFileEx
XPLATAPPX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFile(
    char* utf8File,
    bool forRead,
    IStream** stream);

XPLATAPPX_API HRESULT STDMETHODCALLTYPE CreateStreamOnFileUTF16(
    LPCWSTR utf16File,
    bool forRead,
    IStream** stream);

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
SpecializeUuidOfImpl(IAppxFactory2);
SpecializeUuidOfImpl(IAppxPackageReader);
SpecializeUuidOfImpl(IAppxPackageReader2);
SpecializeUuidOfImpl(IAppxPackageWriter);
SpecializeUuidOfImpl(IAppxPackageWriter2);
//SpecializeUuidOfImpl(IAppxPackageWriter3);
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
//SpecializeUuidOfImpl(IAppxManifestReader6);
SpecializeUuidOfImpl(IAppxManifestOptionalPackageInfo);
SpecializeUuidOfImpl(IAppxManifestMainPackageDependenciesEnumerator);
SpecializeUuidOfImpl(IAppxManifestMainPackageDependency);
SpecializeUuidOfImpl(IAppxManifestPackageId);
//SpecializeUuidOfImpl(IAppxManifestPackageId2);
SpecializeUuidOfImpl(IAppxManifestProperties);
SpecializeUuidOfImpl(IAppxManifestTargetDeviceFamiliesEnumerator);
SpecializeUuidOfImpl(IAppxManifestTargetDeviceFamily);
SpecializeUuidOfImpl(IAppxManifestPackageDependenciesEnumerator);
SpecializeUuidOfImpl(IAppxManifestPackageDependency);
SpecializeUuidOfImpl(IAppxManifestPackageDependency2);
SpecializeUuidOfImpl(IAppxManifestResourcesEnumerator);
SpecializeUuidOfImpl(IAppxManifestDeviceCapabilitiesEnumerator);
SpecializeUuidOfImpl(IAppxManifestCapabilitiesEnumerator);
SpecializeUuidOfImpl(IAppxManifestApplicationsEnumerator);
SpecializeUuidOfImpl(IAppxManifestApplication);
SpecializeUuidOfImpl(IAppxManifestQualifiedResourcesEnumerator);
SpecializeUuidOfImpl(IAppxManifestQualifiedResource);
SpecializeUuidOfImpl(IAppxBundleFactory);
SpecializeUuidOfImpl(IAppxBundleWriter);
SpecializeUuidOfImpl(IAppxBundleWriter2);
//SpecializeUuidOfImpl(IAppxBundleWriter3);
SpecializeUuidOfImpl(IAppxBundleReader);
SpecializeUuidOfImpl(IAppxBundleManifestReader);
SpecializeUuidOfImpl(IAppxBundleManifestReader2);
SpecializeUuidOfImpl(IAppxBundleManifestPackageInfoEnumerator);
SpecializeUuidOfImpl(IAppxBundleManifestPackageInfo);
//SpecializeUuidOfImpl(IAppxBundleManifestPackageInfo2);
SpecializeUuidOfImpl(IAppxBundleManifestOptionalBundleInfoEnumerator);
SpecializeUuidOfImpl(IAppxBundleManifestOptionalBundleInfo);
SpecializeUuidOfImpl(IAppxContentGroupFilesEnumerator);
SpecializeUuidOfImpl(IAppxContentGroup);
SpecializeUuidOfImpl(IAppxContentGroupsEnumerator);
SpecializeUuidOfImpl(IAppxContentGroupMapReader);
SpecializeUuidOfImpl(IAppxSourceContentGroupMapReader);
SpecializeUuidOfImpl(IAppxContentGroupMapWriter);
SpecializeUuidOfImpl(IAppxEncryptionFactory);
SpecializeUuidOfImpl(IAppxEncryptionFactory2);
SpecializeUuidOfImpl(IAppxEncryptionFactory3);
//SpecializeUuidOfImpl(IAppxEncryptionFactory4);
SpecializeUuidOfImpl(IAppxEncryptedPackageWriter);
//SpecializeUuidOfImpl(IAppxEncryptedPackageWriter2);
SpecializeUuidOfImpl(IAppxEncryptedBundleWriter);
SpecializeUuidOfImpl(IAppxEncryptedBundleWriter2);
//SpecializeUuidOfImpl(IAppxPackageEditor);

#endif //__appxpackaging_hpp__