//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
// This header defines the types used by Windows that are not defined in other platforms
#ifndef __appxwindows_hpp__
#define __appxwindows_hpp__

#include <cstdint>
#include <string>
#include <cstring>

#ifdef WIN32
    #define STDMETHODCALLTYPE __stdcall
    #define MSIX_API extern "C" __declspec(dllexport)
    #define DECLSPEC_SELECTANY __declspec(selectany)

    // UNICODE MUST be defined before you include Windows.h if you want the non-ascii versions of APIs (and you do)
    #ifdef UNICODE
    #undef UNICODE
    #endif

    #define UNICODE
    #define NOMINMAX
    #include <windows.h>
    // Windows.h defines max and min, which does NOT play nice at all with std::min / std::max usage from <algorithm>
    #undef max
    #undef min
#else
    // On x86-x64 non-win32 platforms, use SYSTEM V AMD64 ABI calling convention.  This should suffice for Solaris, Linux, BSD,
    // MacOS and anything compiled with GCC or Intel compilers.  On non-x86-x64 architecures, we will use the compiler default
    // See https://en.wikipedia.org/wiki/X86_calling_conventions#List_of_x86_calling_conventions for details
    #ifndef STDMETHODCALLTYPE
    #define STDMETHODCALLTYPE
    #endif

    #undef MSIX_API
    #define MSIX_API extern "C"

    #undef DECLSPEC_SELECTANY
    #define DECLSPEC_SELECTANY

    #ifndef interface
    #define interface struct
    #endif

    #ifndef _WINDOWS_TYPES
    #define _WINDOWS_TYPES

    #ifndef _HRESULT_DEFINED
    #define _HRESULT_DEFINED
    typedef signed long HRESULT;
    #endif

    #ifndef _LONG_DEFINED
    #define _LONG_DEFINED
    typedef signed long LONG;
    #endif

    #ifndef _NTSTATUS_DEFINED
    #define _NTSTATUS_DEFINED
    typedef LONG NTSTATUS;
    #endif

    #ifndef _ULONG_DEFINED
    #define _ULONG_DEFINED
    typedef unsigned long ULONG;
    #endif

    #ifndef _DWORD_DEFINED
    #define _DWORD_DEFINED
    typedef unsigned long DWORD;
    #endif

    #ifndef _LONGLONG_DEFINED
    #define _LONGLONG_DEFINED
    typedef signed long long LONGLONG;
    #endif

    #ifndef _ULONGLONG_DEFINED
    #define _ULONGLONG_DEFINED
    typedef unsigned long long ULONGLONG;
    #endif

    #ifndef _UINT16_DEFINED
    #define _UINT16_DEFINED
    typedef unsigned short UINT16;
    #endif

    #ifndef _UINT32_DEFINED
    #define _UINT32_DEFINED
    typedef unsigned int UINT32;
    #endif

    #ifndef _UINT64_DEFINED
    #define _UINT64_DEFINED
    typedef unsigned long long UINT64;
    #endif

    #ifndef _WCHAR_DEFINED
    #define _WCHAR_DEFINED
    typedef wchar_t WCHAR;
    #endif

    #ifndef _LPWSTR_DEFINED
    #define _LPWSTR_DEFINED
    typedef WCHAR* LPWSTR;
    #endif

    #ifndef _LPCWSTR_DEFINED
    #define _LPCWSTR_DEFINED
    typedef const WCHAR* LPCWSTR;
    #endif

    #ifndef _LPSTR_DEFINED
    #define _LPSTR_DEFINED
    typedef char* LPSTR;
    #endif

    #ifndef _LPCSTR_DEFINED
    #define _LPCSTR_DEFINED
    typedef const char* LPCSTR;
    #endif

    #ifndef _BYTE_DEFINED
    #define _BYTE_DEFINED
    typedef unsigned char BYTE;
    #endif

    #ifndef _BOOL_DEFINED
    #define _BOOL_DEFINED
    typedef int BOOL;
    #endif

    #ifndef _SIZE_T_DEFINED
    #define _SIZE_T_DEFINED
    typedef size_t SIZE_T;
    #endif

    #ifndef _LPVOID_DEFINED
    #define _LPVOID_DEFINED
    typedef void* LPVOID;
    #endif

    #ifndef FALSE
    #define FALSE 0
    #endif

    #ifndef TRUE
    #define TRUE 1
    #endif

    #ifndef _LARGE_INTEGER_DEFINED
    #define _LARGE_INTEGER_DEFINED
    typedef union _LARGE_INTEGER {
        struct {
            ULONG LowPart;
            LONG HighPart;
        } DUMMYSTRUCTNAME;
        struct {
            ULONG LowPart;
            LONG HighPart;
        } u;
        LONGLONG QuadPart;
    } LARGE_INTEGER;
    #endif

    #ifndef _ULARGE_INTEGER_DEFINED
    #define _ULARGE_INTEGER_DEFINED
    typedef union _ULARGE_INTEGER {
        struct {
            ULONG LowPart;
            ULONG HighPart;
        } DUMMYSTRUCTNAME;
        struct {
            ULONG LowPart;
            ULONG HighPart;
        } u;
        ULONGLONG QuadPart;
    } ULARGE_INTEGER;
    #endif

    #ifndef _TAGFILETIME_DEFINED
    #define _TAGFILETIME_DEFINED
    typedef struct tagFILETIME
    {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME;
    #endif

    #endif // _WINDOWS_TYPES

    #ifndef LPOLESTR
    #define LPOLESTR void*
    #endif

    #ifndef IUri
    #define IUri void*
    #endif

    #ifndef EXTERN_C
    #define EXTERN_C extern "C"
    #endif

    #ifndef GUID_DEFINED
    #define GUID_DEFINED
    typedef struct _GUID {
        unsigned long  Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char  Data4[8];
    } GUID;
    #endif

    #ifndef __IID_DEFINED__
    #define __IID_DEFINED__
    typedef GUID IID;
    typedef GUID CLSID;


    #ifndef _REFGUID_DEFINED
    #define _REFGUID_DEFINED
    #define REFGUID const GUID &
    #endif

    #ifndef _REFIID_DEFINED
    #define _REFIID_DEFINED
    #define REFIID const IID &
    #endif

    #endif // !__IID_DEFINED__

    #ifndef S_OK
    #define S_OK 0L
    #define S_FALSE 1L
    #endif

    #ifndef SUCCEEDED
    #define SUCCEEDED(hr) ((((HRESULT)(hr)) == 0) || (((HRESULT)(hr)) == 1))
    #endif

    #ifndef FAILED
    #define FAILED(hr) !SUCCEEDED(hr)
    #endif

    #ifndef FACILITY_WIN32
    #define FACILITY_WIN32 7
    #endif

    #ifndef HRESULT_FROM_WIN32
    #define HRESULT_FROM_WIN32(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))
    #endif

    #ifndef E_NOINTERFACE
    #define E_NOINTERFACE 0x80004002
    #endif

    #ifndef E_INVALIDARG
    #define E_INVALIDARG 0x80070057
    #endif

    #ifndef E_NOTIMPL
    #define E_NOTIMPL 0x80004001
    #endif

    #ifndef E_BOUNDS
    #define E_BOUNDS 0x8000000b
    #endif

    #if !defined (_SYS_GUID_OPERATORS_)
    #define _SYS_GUID_OPERATORS_

    inline int IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
    {
        return !memcmp(&rguid1, &rguid2, sizeof(GUID));
    }

    // Same type, different name
    #define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)

    #if !defined _SYS_GUID_OPERATOR_EQ_ && !defined _NO_SYS_GUID_OPERATOR_EQ_
    #define _SYS_GUID_OPERATOR_EQ_
    // A couple of C++ helpers
    __inline bool operator==(REFGUID guidOne, REFGUID guidOther)
    {
        return !!IsEqualGUID(guidOne, guidOther);
    }

    __inline bool operator!=(REFGUID guidOne, REFGUID guidOther)
    {
        return !(guidOne == guidOther);
    }
    #endif  // _SYS_GUID_OPERATOR_EQ_
    #endif  // _SYS_GUID_OPERATORS_
#endif // #else of #ifdef WIN32

#endif //__appxwindows_hpp__
