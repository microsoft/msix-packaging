#pragma once
#include <windows.h>
#include "AppxPackaging.hpp"
#include <winmeta.h>
#include <string.h>

namespace MsixCoreLib
{
    //
    // Converts a wstring from utf16 to utf8
    //
    // Parameters:
    // utf16string - A utf16 wstring
    // 
    std::string utf16_to_utf8(const std::wstring& utf16string);

    /// Converts a string from utf8 to utf16
    ///
    /// @param utf8string - A utf8 string
    /// @return utf16 string
    std::wstring utf8_to_utf16(const std::string& utf8string);

    // Helper to convert version number to a version number string
    std::wstring ConvertVersionToString(UINT64 version);

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

    /// Determines if a string contains a substring, insensitive to case
    ///
    /// @param string
    /// @param substring
    /// @return true if string contains substring, false otherwise
    bool CaseInsensitiveIsSubString(const std::wstring& string, const std::wstring& substring);

    /// Returns the current user sid as a string.
    ///
    /// @param userSidString - current user sid string
    HRESULT GetCurrentUserSidString(std::wstring & userSidString);

    /// Determines if the currently running OS is Windows 10 RS3 or later.
    /// For this to work correctly, the calling exe needs to have compatibility manifest to allow it to detect windows 10 versions
    /// https://docs.microsoft.com/en-us/windows/desktop/SysInfo/targeting-your-application-at-windows-8-1
    BOOL IsWindows10RS3OrLater();

    /// Determines whether a string ends with a given substring
    ///
    /// @return true if "fullString" ends with the specified "ending"; otherwise false
    bool EndsWith(std::wstring const &fullString, std::wstring const &ending);

    /// Determines whether a given file path corresponds to a package file (.appx, .msix)
    ///
    /// @path -path for some file
    /// @return true if "path" is a package file path ending with ".appx" or "msix"; otherwise false
    bool IsPackageFile(std::wstring const& path);

    /// Determines whether a given file path corresponds to a bundle file (.appxbundle, .msixbundle)
    ///
    /// @path -path for some file
    /// @return true if "path" is a bundle file path ending with ".appxbundle" or "msixbundle"; otherwise false
    bool IsBundleFile(std::wstring const& path);

    /// Replaces all oldchars in input with newchar
    ///
    /// @param input   - The input string that contains the characters to be changed
    /// @param oldchar - Old character that are to be replaced
    /// @param newchar - New character that replaces oldchar
    void replace(std::wstring& input, const wchar_t oldchar, const wchar_t newchar);

    /// Makes a directory, including all parent directories based on the inputted filepath
    ///
    /// @param utf16Path - The filepath to create a directory in utf16
    int mkdirp(std::wstring& utf16Path);

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


    HRESULT GetAttributeValueFromElement(IMsixElement* element, std::wstring attributeName, std::wstring& attributeValue);
    /// The manifest ID is missing the curly braces;
    /// This adds the curly braces to convert it into a proper Guid form.
    std::wstring GuidFromManifestId(std::wstring id);

    HRESULT FileExists(std::wstring file, _Out_ bool &exists);

    HRESULT ConvertLogoToIcon(std::wstring logoPath, std::wstring & iconPath);

    class AutoCoInitialize
    {
    public:
        inline AutoCoInitialize()
            : hr(CO_E_NOTINITIALIZED)
        {
        }

        inline ~AutoCoInitialize()
        {
            if (SUCCEEDED(hr))
            {
                Uninitialize();
            }
        }

        inline HRESULT Initialize(
            _In_ DWORD threadingModel = COINIT_MULTITHREADED)
        {
            HRESULT hr = CoInitializeEx(NULL, threadingModel);
            if (SUCCEEDED(hr))
            {
                this->hr = hr;
            }
            else if (hr == RPC_E_CHANGED_MODE)
            {
                // Thread was already initialized with a different apartment model, don't need to initialize again.
                // But leave this->hr as a FAILED value as we didn't successfully initialize COM so we better not uninitialize it later
                return S_FALSE;
            }

            return hr;
        }

        inline void Uninitialize()
        {
            if (SUCCEEDED(hr))
            {
                CoUninitialize();
                this->hr = CO_E_NOTINITIALIZED;
            }
        }

    private:
        // Actions Not Supported
        AutoCoInitialize(_In_ const AutoCoInitialize&);
        AutoCoInitialize& operator=(_In_ const AutoCoInitialize&);

    private:
        HRESULT hr;
    };

    class Bstr
    {
        BSTR m_bstr;
    public:
        operator BSTR() && = delete;
        operator BSTR() & { return m_bstr; }
        Bstr() { m_bstr = nullptr; }
        Bstr(std::wstring text)
        {
            m_bstr = ::SysAllocStringLen(text.c_str(), static_cast<UINT>(text.length()));
        }
        ~Bstr() { ::SysFreeString(m_bstr); }

        BSTR* AddressOf()
        {
            ::SysFreeString(m_bstr);
            return &m_bstr;
        }

        BSTR& Get() { return m_bstr; }
    };

}