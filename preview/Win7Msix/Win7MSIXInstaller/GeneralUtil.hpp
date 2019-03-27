#pragma once
#include <windows.h>
#include "AppxPackaging.hpp"
#include <TraceLoggingProvider.h>
#include <winmeta.h>
#include <string.h>

TRACELOGGING_DECLARE_PROVIDER(g_MsixTraceLoggingProvider);

// Definition of function to return error if failed
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define RETURN_IF_FAILED(a) \
{                       \
    HRESULT __hr = a;   \
    if (FAILED(__hr))   \
    {   \
        TraceLoggingWrite(g_MsixTraceLoggingProvider, \
            "RETURN_IF_FAILED", \
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR), \
            TraceLoggingValue(#a, "Code"), \
            TraceLoggingHResult(__hr, "HR"), \
            TraceLoggingUInt32(__LINE__, "Line"), \
            TraceLoggingValue(__FILENAME__, "Filename")); \
        return __hr; \
    }  \
}

/// Converts a wstring from utf16 to utf8
///
/// @param utf16string - A utf16 wstring
/// @return utf8 string
std::string utf16_to_utf8(const std::wstring& utf16string);

/// Converts a string from utf8 to utf16
///
/// @param utf8string - A utf8 string
/// @return utf16 string
std::wstring utf8_to_utf16(const std::string& utf8string);

/// Helper to convert version number to a version string of the form a.b.c.d
///
/// @param version - version number
/// @return a.b.c.d string representation of version
std::wstring ConvertVersionToString(UINT64 version);

/// Helper to get string resource
///
/// @param resourceId - resource ID, these should be listed in resource.h
/// @return string for the resource, resolved from the stringtable defined in Win7MsixInstaller.rc
std::wstring GetStringResource(UINT resourceId);

/// Converts a packageFullName (i.e. examplePackageName_1.0.0.0_x64_resourceId_8wekyb3d8bbwe) 
/// into a packageFamilyName (i.e. examplePackageName_8wekyb3d8bbwe)
///
/// @param fullName - the packageFullName, assumed to be properly formatted and not validated.
/// @return packageFamilyName for the packageFullName
std::wstring GetFamilyNameFromFullName(const std::wstring& fullName);

/// Determines if two strings are case-insensitive equals
///
/// @param left - one of the two strings
/// @param right - the other of the two strings
/// @return true if the strings equal, false otherwise
bool CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right);

//
// A designated memory allocator
// 
// Parameters:
// cb - The size of memory
//
inline LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb) { return std::malloc(cb); }

//
// A designated memory freeing method
//
// Parameters:
// pv - A pointer to the file to release
//
inline void STDMETHODCALLTYPE MyFree(LPVOID pv) { std::free(pv); }

//
// Stripped down ComPtr class provided for those platforms that do not already have a ComPtr class.
//
template <class T>
class ComPtr
{
    public:
        // default ctor
        ComPtr() = default;
        ComPtr(T* ptr) : m_ptr(ptr) { InternalAddRef(); }

        ~ComPtr() { InternalRelease(); }
        inline T* operator->() const { return m_ptr; }
        inline T* Get() const { return m_ptr; }

        inline T** operator&()
        {
            InternalRelease();
            return &m_ptr;
        }

        inline T* operator=(__in_opt T* ptr) throw()
        {
            InternalRelease();
            m_ptr = ptr;
            InternalAddRef();
            return m_ptr;
        }

        void Release() { InternalRelease(); }

    protected:
        T * m_ptr = nullptr;

        inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
        inline void InternalRelease()
        {
            T* temp = m_ptr;
            if (temp)
            {
                m_ptr = nullptr;
                temp->Release();
            }
        }
};

template <class T>
class AutoPtr
{
public:
    AutoPtr() = default;
    AutoPtr(T * ptr) : m_ptr(ptr) {}

    ~AutoPtr() { delete m_ptr; }
    void Free() { delete m_ptr; m_ptr = 0; }

    inline T* Detach()
    {
        T* old = m_ptr;
        m_ptr = 0;
        return old;
    }

    inline operator const T* () const
    {
        return (T*)m_ptr;
    }

    inline operator T* ()
    {
        return (T*)m_ptr;
    }

    inline const T& operator*() const
    {
        return *m_ptr;
    }
    inline T& operator*()
    {
        return *m_ptr;
    }

    inline const T* Get() const
    {
        return m_ptr;
    }

    inline T* Get()
    {
        return m_ptr;
    }

    inline T** operator&()
    {
        return &m_ptr;
    }

    inline T** AddressOf()
    {
        return &m_ptr;
    }

    inline T** FreeAndAddressOf()
    {
        Free();
        return &m_ptr;
    }

    inline const T* operator->() const
    {
        return m_ptr;
    }
    inline T* operator->()
    {
        return m_ptr;
    }

    inline T* operator=(T* ptr)
    {
        if (m_ptr != ptr)
        {
            delete m_ptr;
            m_ptr = ptr;
        }
        return m_ptr;
    }

protected:
    T* m_ptr = nullptr;
};

//
// Helper class to free string buffers obtained from the packaging APIs.
//
template<typename T>
class Text
{
    public:
        T** operator&() { return &content; }
        ~Text() { Cleanup(); }
        T* Get() { return content; }

        T* content = nullptr;
    protected:
        void Cleanup() { if (content) { MyFree(content); content = nullptr; } }
};


//
// Helper class to free string buffers created using OLE memory allocator.
//
template<typename T>
class TextOle
{
    public:
        T** operator&() { return &content; }
        ~TextOle() { Cleanup(); }
        T* Get() { return content; }

        T* content = nullptr;
    protected:
        void Cleanup() { if (content) { CoTaskMemFree(content); content = nullptr; } }
};

