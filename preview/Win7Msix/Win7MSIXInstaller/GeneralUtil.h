#pragma once
#include <windows.h>
#include "AppxPackaging.hpp"

// Definition of function to return error if failed
#define RETURN_IF_FAILED(a) \
    {                       \
        HRESULT __hr = a;   \
        if (FAILED(__hr))   \
        {   return __hr; }  \
    }

//
// Converts a wstring from utf16 to utf8
//
// Parameters:
// utf16string - A utf16 wstring
// 
std::string utf16_to_utf8(const std::wstring& utf16string);

//
// Converts a string from utf8 to utf16
//
// Parameters:
// utf8string - A utf8 string
// 
std::wstring utf8_to_utf16(const std::string& utf8string);

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
// Tracks the state of the current parse operation as well as implements input validation
//
struct State
{
    bool SkipManifestValidation()
    {
        validationOptions = static_cast<MSIX_VALIDATION_OPTION>(validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPAPPXMANIFEST);
        return true;
    }

    bool SkipSignature()
    {
        validationOptions = static_cast<MSIX_VALIDATION_OPTION>(validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE);
        return true;
    }

    bool AllowSignatureOriginUnknown()
    {
        validationOptions = static_cast<MSIX_VALIDATION_OPTION>(validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN);
        return true;
    }

    bool SetPackageName(const std::string& name)
    {
        if (!packageName.empty() || name.empty()) { return false; }
        packageName = utf8_to_utf16(name);
        return true;
    }

    bool SetUninstallXml(const std::string& uninstallXml)
    {
        if (!m_uninstallXml.empty() || uninstallXml.empty()) { return false; }
        m_uninstallXml = utf8_to_utf16(uninstallXml);
        return true;
    }

    bool SetDirectoryName(const std::string& name)
    {
        if (!directoryName.empty() || name.empty()) { return false; }
        directoryName = utf8_to_utf16(name);
        return true;
    }

    bool SetRecursiveUnpack()
    {
        recursiveUnpack = true;
        return true;
    }

    bool IsUninstall()
    {
        return !m_uninstallXml.empty();
    }

    std::wstring packageName;
    std::wstring directoryName;
    std::wstring m_uninstallXml;
    bool recursiveUnpack = false;
    MSIX_VALIDATION_OPTION validationOptions = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
};
