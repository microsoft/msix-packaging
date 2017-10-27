// This header defines the types used by Windows that are not defined in other platforms
#ifndef __appxwindows_hpp__
#define __appxwindows_hpp__

#include <cstdint>
#include <string>
#include <cstring>

#ifndef WIN32
// On x86-x64 non-win32 platforms, use SYSTEM V AMD64 ABI calling convention.  This should suffice for Solaris, Linux, BSD,
// MacOS and anything compiled with GCC or Intel compilers.  On non-x86-x64 architecures, we will use the compiler default
// See https://en.wikipedia.org/wiki/X86_calling_conventions#List_of_x86_calling_conventions for details
    #define STDMETHODCALLTYPE
#else
    #define STDMETHODCALLTYPE __stdcall
#endif

#ifndef MIDL_INTERFACE
#define MIDL_INTERFACE(i)
#endif

#ifndef interface
#define interface struct
#endif

typedef unsigned long HRESULT;
typedef signed long LONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef wchar_t LPWSTR;
typedef const wchar_t LPCWSTR;
typedef char BYTE;
typedef bool BOOL;

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

typedef struct tagFILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

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
#define S_OK 0
#endif

#ifndef SUCCEDED
#define SUCCEDED(hr) hr == 0
#endif

#if !defined (_SYS_GUID_OPERATORS_)
#define _SYS_GUID_OPERATORS_

// Faster (but makes code fatter) inline version...use sparingly
inline int IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (
        ((unsigned long *)&rguid1)[0] == ((unsigned long *)&rguid2)[0] &&
        ((unsigned long *)&rguid1)[1] == ((unsigned long *)&rguid2)[1] &&
        ((unsigned long *)&rguid1)[2] == ((unsigned long *)&rguid2)[2] &&
        ((unsigned long *)&rguid1)[3] == ((unsigned long *)&rguid2)[3]);
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

#endif //__appxwindows_hpp__