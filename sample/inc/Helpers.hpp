//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
// This is a helper file for the samples. Includes useful RAII wrappers, flow control
// macros and other common functions used by the samples.

namespace MsixSample { namespace Helper 

    // Stripped down ComPtr provided for those platforms that do not already have a ComPtr class.
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
        {   InternalRelease();
            return &m_ptr;
        }

    protected:
        T* m_ptr = nullptr;

        inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
        inline void InternalRelease()
        {
            T* temp = m_ptr;
            if (temp)
            {   m_ptr = nullptr;
                temp->Release();
            }
        }
    };

    // Helper class to free string buffers obtained from the packaging APIs.
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

    // Allocators
    LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
    void STDMETHODCALLTYPE MyFree(LPVOID pv)        { std::free(pv); }

    // Useful string convertions functions
    std::string utf16_to_utf8(const std::wstring& utf16string)
    {
        auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        std::string result(converted.begin(), converted.end());
        return result;
    }

    std::wstring utf8_to_utf16(const std::string& utf8string)
    {
        // see https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
        #ifdef WIN32
        auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
        #else
        auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
        #endif
        std::wstring result(converted.begin(), converted.end());
        return result;
    }

    // Directory helpers
    #ifdef WIN32
    // TODO: paths coming in SHOULD have platform-appropriate path separators
    void replace(std::wstring& input, const wchar_t oldchar, const wchar_t newchar)
    {
        std::size_t found = input.find_first_of(oldchar);
        while (found != std::string::npos)
        {
            input[found] = newchar;
            found = input.find_first_of(oldchar, found+1);
        }
    }

    int mkdirp(std::wstring& utf16Path)
    {
        replace(utf16Path, L'/', L'\\');
        for (std::size_t i = 0; i < utf16Path.size(); i++)
        {
            if (utf16Path[i] == L'\0')
            {
                break;
            }
            else if (utf16Path[i] == L'\\')
            {
                // Temporarily set string to terminate at the '\' character
                // to obtain name of the subdirectory to create
                utf16Path[i] = L'\0';

                if (!CreateDirectory(utf16Path.c_str(), nullptr))
                {
                    int lastError = static_cast<int>(GetLastError());

                    // It is normal for CreateDirectory to fail if the subdirectory
                    // already exists.  Other errors should not be ignored.
                    if (lastError != ERROR_ALREADY_EXISTS)
                    {
                        return lastError;
                    }
                }
                // Restore original string
                utf16Path[i] = L'\\'; /* TODO: paths coming in SHOULD have platform-appropriate path separators */
            }
        }
        return 0;
    }
    #else
        // not all POSIX implementations provide an implementation of mkdirp
        int mkdirp(std::wstring& utf16Path)
        {
            std::string utf8Path = utf16_to_utf8(utf16Path);
            auto lastSlash = utf8Path.find_last_of("/");
            std::string path = utf8Path.substr(0, lastSlash);
            char* p = const_cast<char*>(path.c_str());
            if (*p == '/') { p++; }
            while (*p != '\0')
            {
                while (*p != '\0' && *p != '/') { p++; }

                char v = *p;
                *p = '\0';
                if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
                {
                    return errno;
                }
                *p = v;
                p++;
            }
            return 0;
        }
    #endif
} }

// Flow control macros
#define RETURN_IF_FAILED(a) \
    {   HRESULT __hr = a;   \
        if (FAILED(__hr))   \
        {   return __hr; }  \
    }
