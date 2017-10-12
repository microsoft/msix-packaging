

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for Source.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Source_h__
#define __Source_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IXplatAppxFactory_FWD_DEFINED__
#define __IXplatAppxFactory_FWD_DEFINED__
typedef interface IXplatAppxFactory IXplatAppxFactory;

#endif 	/* __IXplatAppxFactory_FWD_DEFINED__ */


#ifndef __IXplatAppxPackage_FWD_DEFINED__
#define __IXplatAppxPackage_FWD_DEFINED__
typedef interface IXplatAppxPackage IXplatAppxPackage;

#endif 	/* __IXplatAppxPackage_FWD_DEFINED__ */


#ifndef __IXplatAppxFile_FWD_DEFINED__
#define __IXplatAppxFile_FWD_DEFINED__
typedef interface IXplatAppxFile IXplatAppxFile;

#endif 	/* __IXplatAppxFile_FWD_DEFINED__ */


#ifndef __IEnumXplatAppxFile_FWD_DEFINED__
#define __IEnumXplatAppxFile_FWD_DEFINED__
typedef interface IEnumXplatAppxFile IEnumXplatAppxFile;

#endif 	/* __IEnumXplatAppxFile_FWD_DEFINED__ */


#ifndef __IXplatAppxBlockMapReader_FWD_DEFINED__
#define __IXplatAppxBlockMapReader_FWD_DEFINED__
typedef interface IXplatAppxBlockMapReader IXplatAppxBlockMapReader;

#endif 	/* __IXplatAppxBlockMapReader_FWD_DEFINED__ */


#ifndef __IXplatAppxBlockMapFile_FWD_DEFINED__
#define __IXplatAppxBlockMapFile_FWD_DEFINED__
typedef interface IXplatAppxBlockMapFile IXplatAppxBlockMapFile;

#endif 	/* __IXplatAppxBlockMapFile_FWD_DEFINED__ */


#ifndef __IEnumXplatAppxBlockMapFile_FWD_DEFINED__
#define __IEnumXplatAppxBlockMapFile_FWD_DEFINED__
typedef interface IEnumXplatAppxBlockMapFile IEnumXplatAppxBlockMapFile;

#endif 	/* __IEnumXplatAppxBlockMapFile_FWD_DEFINED__ */


#ifndef __IXplatAppxBlockMapBlock_FWD_DEFINED__
#define __IXplatAppxBlockMapBlock_FWD_DEFINED__
typedef interface IXplatAppxBlockMapBlock IXplatAppxBlockMapBlock;

#endif 	/* __IXplatAppxBlockMapBlock_FWD_DEFINED__ */


#ifndef __IEnumXplatAppxBlockMapBlock_FWD_DEFINED__
#define __IEnumXplatAppxBlockMapBlock_FWD_DEFINED__
typedef interface IEnumXplatAppxBlockMapBlock IEnumXplatAppxBlockMapBlock;

#endif 	/* __IEnumXplatAppxBlockMapBlock_FWD_DEFINED__ */


#ifndef __XplatAppxFactory_FWD_DEFINED__
#define __XplatAppxFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class XplatAppxFactory XplatAppxFactory;
#else
typedef struct XplatAppxFactory XplatAppxFactory;
#endif /* __cplusplus */

#endif 	/* __XplatAppxFactory_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_Source_0000_0000 */
/* [local] */ 

typedef /* [v1_enum] */ 
enum XPLATAPPX_VALIDATION_OPTION
    {
        XPLATAPPX_VALIDATION_OPTION_FULL	= 0x1,
        XPLATAPPX_VALIDATION_OPTION_SKIPSIGNATUREORIGIN	= 0x2,
        XPLATAPPX_VALIDATION_OPTION_SKIPAPPXMANIFEST	= 0x4
    } 	XPLATAPPX_VALIDATION_OPTION;

typedef /* [v1_enum] */ 
enum XPLATAPPX_PACKUNPACK_OPTION
    {
        XPLATAPPX_PACKUNPACK_OPTION_NONE	= 0,
        XPLATAPPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER	= 0x1
    } 	XPLATAPPX_PACKUNPACK_OPTION;

#if defined PLATFORM_APPLE
    #define XPLATAPPX_CONV
#elif defined PLATFORM_ANDROID
    #define XPLATAPPX_CONV
#elif defined PLATFORM_LINUX
    #define XPLATAPPX_CONV
#else // WIN32
    #define XPLATAPPX_CONV __stdcall
#endif
#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus
    HRESULT XPLATAPPX_CONV PackAppx(
        XPLATAPPX_PACKUNPACK_OPTION packUnpackOptions,
        XPLATAPPX_VALIDATION_OPTION validationOption,
        char* utf8FolderToPack,
        char* utf8CertificatePath,
        char* utf8Destination
    );
    HRESULT XPLATAPPX_CONV UnpackAppx(
        XPLATAPPX_PACKUNPACK_OPTION packUnpackOptions,
        XPLATAPPX_VALIDATION_OPTION validationOption,
        char* utf8TargetFolder,
        char* utf8Destination
    );
    typedef void* XPLATAPPX_CONV COTASKMEMALLOC(size_t cb);
    typedef void XPLATAPPX_CONV COTASKMEMFREE(void* pv);
    HRESULT XPLATAPPX_CONV XplatAppxInitialize(COTASKMEMALLOC* memalloc, COTASKMEMFREE* memfree);
#ifdef __cplusplus
}
#endif // __cplusplus
typedef /* [v1_enum] */ 
enum XPLATAPPX_FOOTPRINT_FILE_TYPE
    {
        XPLATAPPX_FOOTPRINT_FILE_TYPE_MANIFEST	= 0,
        XPLATAPPX_FOOTPRINT_FILE_TYPE_BLOCKMAP	= 1,
        XPLATAPPX_FOOTPRINT_FILE_TYPE_SIGNATURE	= 2,
        XPLATAPPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY	= 3,
        XPLATAPPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP	= 4
    } 	XPLATAPPX_FOOTPRINT_FILE_TYPE;











extern RPC_IF_HANDLE __MIDL_itf_Source_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_Source_0000_0000_v0_0_s_ifspec;

#ifndef __IXplatAppxFactory_INTERFACE_DEFINED__
#define __IXplatAppxFactory_INTERFACE_DEFINED__

/* interface IXplatAppxFactory */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e0a98297-e0ed-4111-a0ee-dd2a461787b0")
    IXplatAppxFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePackageFromFile( 
            /* [string][in] */ LPWSTR fileName,
            /* [in] */ XPLATAPPX_VALIDATION_OPTION validationOption,
            /* [out] */ IXplatAppxPackage **packageReader) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxFactory * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePackageFromFile )( 
            IXplatAppxFactory * This,
            /* [string][in] */ LPWSTR fileName,
            /* [in] */ XPLATAPPX_VALIDATION_OPTION validationOption,
            /* [out] */ IXplatAppxPackage **packageReader);
        
        END_INTERFACE
    } IXplatAppxFactoryVtbl;

    interface IXplatAppxFactory
    {
        CONST_VTBL struct IXplatAppxFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxFactory_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxFactory_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxFactory_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxFactory_CreatePackageFromFile(This,fileName,validationOption,packageReader)	\
    ( (This)->lpVtbl -> CreatePackageFromFile(This,fileName,validationOption,packageReader) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxFactory_INTERFACE_DEFINED__ */


#ifndef __IXplatAppxPackage_INTERFACE_DEFINED__
#define __IXplatAppxPackage_INTERFACE_DEFINED__

/* interface IXplatAppxPackage */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxPackage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92a4fb80-65f3-4772-a4e3-99efa2ede6c9")
    IXplatAppxPackage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlockMap( 
            /* [out] */ IXplatAppxBlockMapReader **blockMapReader) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFootprintFile( 
            /* [in] */ XPLATAPPX_FOOTPRINT_FILE_TYPE type,
            /* [out] */ IXplatAppxFile **file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFile( 
            /* [string][in] */ LPCWSTR fileName,
            /* [out] */ IXplatAppxFile **file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFiles( 
            /* [out] */ IEnumXplatAppxFile **files) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxPackageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxPackage * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxPackage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxPackage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBlockMap )( 
            IXplatAppxPackage * This,
            /* [out] */ IXplatAppxBlockMapReader **blockMapReader);
        
        HRESULT ( STDMETHODCALLTYPE *GetFootprintFile )( 
            IXplatAppxPackage * This,
            /* [in] */ XPLATAPPX_FOOTPRINT_FILE_TYPE type,
            /* [out] */ IXplatAppxFile **file);
        
        HRESULT ( STDMETHODCALLTYPE *GetFile )( 
            IXplatAppxPackage * This,
            /* [string][in] */ LPCWSTR fileName,
            /* [out] */ IXplatAppxFile **file);
        
        HRESULT ( STDMETHODCALLTYPE *GetFiles )( 
            IXplatAppxPackage * This,
            /* [out] */ IEnumXplatAppxFile **files);
        
        END_INTERFACE
    } IXplatAppxPackageVtbl;

    interface IXplatAppxPackage
    {
        CONST_VTBL struct IXplatAppxPackageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxPackage_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxPackage_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxPackage_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxPackage_GetBlockMap(This,blockMapReader)	\
    ( (This)->lpVtbl -> GetBlockMap(This,blockMapReader) ) 

#define IXplatAppxPackage_GetFootprintFile(This,type,file)	\
    ( (This)->lpVtbl -> GetFootprintFile(This,type,file) ) 

#define IXplatAppxPackage_GetFile(This,fileName,file)	\
    ( (This)->lpVtbl -> GetFile(This,fileName,file) ) 

#define IXplatAppxPackage_GetFiles(This,files)	\
    ( (This)->lpVtbl -> GetFiles(This,files) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxPackage_INTERFACE_DEFINED__ */


#ifndef __IXplatAppxFile_INTERFACE_DEFINED__
#define __IXplatAppxFile_INTERFACE_DEFINED__

/* interface IXplatAppxFile */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("abfaa51a-4fa2-4235-8f28-736a25060e95")
    IXplatAppxFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetContentType( 
            /* [string][out] */ LPWSTR *contentType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [string][out] */ LPWSTR *fileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ UINT64 *size) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStream( 
            /* [out] */ IStream **stream) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxFile * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetContentType )( 
            IXplatAppxFile * This,
            /* [string][out] */ LPWSTR *contentType);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IXplatAppxFile * This,
            /* [string][out] */ LPWSTR *fileName);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IXplatAppxFile * This,
            /* [out] */ UINT64 *size);
        
        HRESULT ( STDMETHODCALLTYPE *GetStream )( 
            IXplatAppxFile * This,
            /* [out] */ IStream **stream);
        
        END_INTERFACE
    } IXplatAppxFileVtbl;

    interface IXplatAppxFile
    {
        CONST_VTBL struct IXplatAppxFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxFile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxFile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxFile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxFile_GetContentType(This,contentType)	\
    ( (This)->lpVtbl -> GetContentType(This,contentType) ) 

#define IXplatAppxFile_GetName(This,fileName)	\
    ( (This)->lpVtbl -> GetName(This,fileName) ) 

#define IXplatAppxFile_GetSize(This,size)	\
    ( (This)->lpVtbl -> GetSize(This,size) ) 

#define IXplatAppxFile_GetStream(This,stream)	\
    ( (This)->lpVtbl -> GetStream(This,stream) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxFile_INTERFACE_DEFINED__ */


#ifndef __IEnumXplatAppxFile_INTERFACE_DEFINED__
#define __IEnumXplatAppxFile_INTERFACE_DEFINED__

/* interface IEnumXplatAppxFile */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IEnumXplatAppxFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71631e46-7c85-4796-8fe3-62e16dde4342")
    IEnumXplatAppxFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent( 
            /* [out] */ IXplatAppxFile **file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent( 
            /* [out] */ boolean *hasCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveNext( 
            /* [out] */ boolean *hasNext) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEnumXplatAppxFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumXplatAppxFile * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumXplatAppxFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumXplatAppxFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrent )( 
            IEnumXplatAppxFile * This,
            /* [out] */ IXplatAppxFile **file);
        
        HRESULT ( STDMETHODCALLTYPE *GetHasCurrent )( 
            IEnumXplatAppxFile * This,
            /* [out] */ boolean *hasCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            IEnumXplatAppxFile * This,
            /* [out] */ boolean *hasNext);
        
        END_INTERFACE
    } IEnumXplatAppxFileVtbl;

    interface IEnumXplatAppxFile
    {
        CONST_VTBL struct IEnumXplatAppxFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumXplatAppxFile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEnumXplatAppxFile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEnumXplatAppxFile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEnumXplatAppxFile_GetCurrent(This,file)	\
    ( (This)->lpVtbl -> GetCurrent(This,file) ) 

#define IEnumXplatAppxFile_GetHasCurrent(This,hasCurrent)	\
    ( (This)->lpVtbl -> GetHasCurrent(This,hasCurrent) ) 

#define IEnumXplatAppxFile_MoveNext(This,hasNext)	\
    ( (This)->lpVtbl -> MoveNext(This,hasNext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEnumXplatAppxFile_INTERFACE_DEFINED__ */


#ifndef __IXplatAppxBlockMapReader_INTERFACE_DEFINED__
#define __IXplatAppxBlockMapReader_INTERFACE_DEFINED__

/* interface IXplatAppxBlockMapReader */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxBlockMapReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("efe725b7-e7cd-4211-ba61-1716996d0384")
    IXplatAppxBlockMapReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFile( 
            /* [string][in] */ LPCWSTR filename,
            /* [out] */ IXplatAppxBlockMapFile **file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFiles( 
            /* [out] */ IEnumXplatAppxBlockMapFile **files) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStream( 
            /* [out] */ IStream **blockMapStream) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxBlockMapReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxBlockMapReader * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxBlockMapReader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxBlockMapReader * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFile )( 
            IXplatAppxBlockMapReader * This,
            /* [string][in] */ LPCWSTR filename,
            /* [out] */ IXplatAppxBlockMapFile **file);
        
        HRESULT ( STDMETHODCALLTYPE *GetFiles )( 
            IXplatAppxBlockMapReader * This,
            /* [out] */ IEnumXplatAppxBlockMapFile **files);
        
        HRESULT ( STDMETHODCALLTYPE *GetStream )( 
            IXplatAppxBlockMapReader * This,
            /* [out] */ IStream **blockMapStream);
        
        END_INTERFACE
    } IXplatAppxBlockMapReaderVtbl;

    interface IXplatAppxBlockMapReader
    {
        CONST_VTBL struct IXplatAppxBlockMapReaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxBlockMapReader_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxBlockMapReader_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxBlockMapReader_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxBlockMapReader_GetFile(This,filename,file)	\
    ( (This)->lpVtbl -> GetFile(This,filename,file) ) 

#define IXplatAppxBlockMapReader_GetFiles(This,files)	\
    ( (This)->lpVtbl -> GetFiles(This,files) ) 

#define IXplatAppxBlockMapReader_GetStream(This,blockMapStream)	\
    ( (This)->lpVtbl -> GetStream(This,blockMapStream) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxBlockMapReader_INTERFACE_DEFINED__ */


#ifndef __IXplatAppxBlockMapFile_INTERFACE_DEFINED__
#define __IXplatAppxBlockMapFile_INTERFACE_DEFINED__

/* interface IXplatAppxBlockMapFile */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxBlockMapFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("276e949a-3517-4bcb-8037-a34f728b2485")
    IXplatAppxBlockMapFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBlocks( 
            /* [out] */ IEnumXplatAppxBlockMapBlock **blocks) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize( 
            /* [out] */ UINT64 *localFileHeaderSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [string][out] */ LPWSTR *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUncompressedSize( 
            /* [out] */ UINT64 *size) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateFileHash( 
            /* [in] */ IStream *fileStream,
            /* [out] */ boolean *isValid) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxBlockMapFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxBlockMapFile * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxBlockMapFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxBlockMapFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBlocks )( 
            IXplatAppxBlockMapFile * This,
            /* [out] */ IEnumXplatAppxBlockMapBlock **blocks);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocalFileHeaderSize )( 
            IXplatAppxBlockMapFile * This,
            /* [out] */ UINT64 *localFileHeaderSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IXplatAppxBlockMapFile * This,
            /* [string][out] */ LPWSTR *name);
        
        HRESULT ( STDMETHODCALLTYPE *GetUncompressedSize )( 
            IXplatAppxBlockMapFile * This,
            /* [out] */ UINT64 *size);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateFileHash )( 
            IXplatAppxBlockMapFile * This,
            /* [in] */ IStream *fileStream,
            /* [out] */ boolean *isValid);
        
        END_INTERFACE
    } IXplatAppxBlockMapFileVtbl;

    interface IXplatAppxBlockMapFile
    {
        CONST_VTBL struct IXplatAppxBlockMapFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxBlockMapFile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxBlockMapFile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxBlockMapFile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxBlockMapFile_GetBlocks(This,blocks)	\
    ( (This)->lpVtbl -> GetBlocks(This,blocks) ) 

#define IXplatAppxBlockMapFile_GetLocalFileHeaderSize(This,localFileHeaderSize)	\
    ( (This)->lpVtbl -> GetLocalFileHeaderSize(This,localFileHeaderSize) ) 

#define IXplatAppxBlockMapFile_GetName(This,name)	\
    ( (This)->lpVtbl -> GetName(This,name) ) 

#define IXplatAppxBlockMapFile_GetUncompressedSize(This,size)	\
    ( (This)->lpVtbl -> GetUncompressedSize(This,size) ) 

#define IXplatAppxBlockMapFile_ValidateFileHash(This,fileStream,isValid)	\
    ( (This)->lpVtbl -> ValidateFileHash(This,fileStream,isValid) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxBlockMapFile_INTERFACE_DEFINED__ */


#ifndef __IEnumXplatAppxBlockMapFile_INTERFACE_DEFINED__
#define __IEnumXplatAppxBlockMapFile_INTERFACE_DEFINED__

/* interface IEnumXplatAppxBlockMapFile */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IEnumXplatAppxBlockMapFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7ffe86b2-5108-4e91-856f-9fbecc728944")
    IEnumXplatAppxBlockMapFile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent( 
            /* [out] */ IXplatAppxBlockMapFile **file) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent( 
            /* [out] */ boolean *hasCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveNext( 
            /* [out] */ boolean *hasCurrent) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEnumXplatAppxBlockMapFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumXplatAppxBlockMapFile * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumXplatAppxBlockMapFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumXplatAppxBlockMapFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrent )( 
            IEnumXplatAppxBlockMapFile * This,
            /* [out] */ IXplatAppxBlockMapFile **file);
        
        HRESULT ( STDMETHODCALLTYPE *GetHasCurrent )( 
            IEnumXplatAppxBlockMapFile * This,
            /* [out] */ boolean *hasCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            IEnumXplatAppxBlockMapFile * This,
            /* [out] */ boolean *hasCurrent);
        
        END_INTERFACE
    } IEnumXplatAppxBlockMapFileVtbl;

    interface IEnumXplatAppxBlockMapFile
    {
        CONST_VTBL struct IEnumXplatAppxBlockMapFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumXplatAppxBlockMapFile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEnumXplatAppxBlockMapFile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEnumXplatAppxBlockMapFile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEnumXplatAppxBlockMapFile_GetCurrent(This,file)	\
    ( (This)->lpVtbl -> GetCurrent(This,file) ) 

#define IEnumXplatAppxBlockMapFile_GetHasCurrent(This,hasCurrent)	\
    ( (This)->lpVtbl -> GetHasCurrent(This,hasCurrent) ) 

#define IEnumXplatAppxBlockMapFile_MoveNext(This,hasCurrent)	\
    ( (This)->lpVtbl -> MoveNext(This,hasCurrent) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEnumXplatAppxBlockMapFile_INTERFACE_DEFINED__ */


#ifndef __IXplatAppxBlockMapBlock_INTERFACE_DEFINED__
#define __IXplatAppxBlockMapBlock_INTERFACE_DEFINED__

/* interface IXplatAppxBlockMapBlock */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IXplatAppxBlockMapBlock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a2f2da6d-12c8-42ba-9d95-1f6c3d60b22c")
    IXplatAppxBlockMapBlock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHash( 
            /* [out] */ UINT32 *bufferSize,
            /* [size_is][size_is][out] */ byte **buffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompressedSize( 
            /* [out] */ UINT32 *size) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IXplatAppxBlockMapBlockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXplatAppxBlockMapBlock * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXplatAppxBlockMapBlock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXplatAppxBlockMapBlock * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHash )( 
            IXplatAppxBlockMapBlock * This,
            /* [out] */ UINT32 *bufferSize,
            /* [size_is][size_is][out] */ byte **buffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompressedSize )( 
            IXplatAppxBlockMapBlock * This,
            /* [out] */ UINT32 *size);
        
        END_INTERFACE
    } IXplatAppxBlockMapBlockVtbl;

    interface IXplatAppxBlockMapBlock
    {
        CONST_VTBL struct IXplatAppxBlockMapBlockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXplatAppxBlockMapBlock_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IXplatAppxBlockMapBlock_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IXplatAppxBlockMapBlock_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IXplatAppxBlockMapBlock_GetHash(This,bufferSize,buffer)	\
    ( (This)->lpVtbl -> GetHash(This,bufferSize,buffer) ) 

#define IXplatAppxBlockMapBlock_GetCompressedSize(This,size)	\
    ( (This)->lpVtbl -> GetCompressedSize(This,size) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXplatAppxBlockMapBlock_INTERFACE_DEFINED__ */


#ifndef __IEnumXplatAppxBlockMapBlock_INTERFACE_DEFINED__
#define __IEnumXplatAppxBlockMapBlock_INTERFACE_DEFINED__

/* interface IEnumXplatAppxBlockMapBlock */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IEnumXplatAppxBlockMapBlock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8a904b5e-b4fb-4e6d-a6b3-187064af7797")
    IEnumXplatAppxBlockMapBlock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrent( 
            /* [out] */ IXplatAppxBlockMapBlock **block) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHasCurrent( 
            /* [out] */ boolean *hasCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveNext( 
            /* [out] */ boolean *hasNext) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEnumXplatAppxBlockMapBlockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumXplatAppxBlockMapBlock * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumXplatAppxBlockMapBlock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumXplatAppxBlockMapBlock * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrent )( 
            IEnumXplatAppxBlockMapBlock * This,
            /* [out] */ IXplatAppxBlockMapBlock **block);
        
        HRESULT ( STDMETHODCALLTYPE *GetHasCurrent )( 
            IEnumXplatAppxBlockMapBlock * This,
            /* [out] */ boolean *hasCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            IEnumXplatAppxBlockMapBlock * This,
            /* [out] */ boolean *hasNext);
        
        END_INTERFACE
    } IEnumXplatAppxBlockMapBlockVtbl;

    interface IEnumXplatAppxBlockMapBlock
    {
        CONST_VTBL struct IEnumXplatAppxBlockMapBlockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumXplatAppxBlockMapBlock_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEnumXplatAppxBlockMapBlock_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEnumXplatAppxBlockMapBlock_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEnumXplatAppxBlockMapBlock_GetCurrent(This,block)	\
    ( (This)->lpVtbl -> GetCurrent(This,block) ) 

#define IEnumXplatAppxBlockMapBlock_GetHasCurrent(This,hasCurrent)	\
    ( (This)->lpVtbl -> GetHasCurrent(This,hasCurrent) ) 

#define IEnumXplatAppxBlockMapBlock_MoveNext(This,hasNext)	\
    ( (This)->lpVtbl -> MoveNext(This,hasNext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEnumXplatAppxBlockMapBlock_INTERFACE_DEFINED__ */



#ifndef __XPLATAPPXPACKAGING_LIBRARY_DEFINED__
#define __XPLATAPPXPACKAGING_LIBRARY_DEFINED__

/* library XPLATAPPXPACKAGING */
/* [lcid][version][uuid] */ 


EXTERN_C const IID LIBID_XPLATAPPXPACKAGING;

EXTERN_C const CLSID CLSID_XplatAppxFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("025ffadb-133c-4b95-989a-fe77a3b0f18a")
XplatAppxFactory;
#endif
#endif /* __XPLATAPPXPACKAGING_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


