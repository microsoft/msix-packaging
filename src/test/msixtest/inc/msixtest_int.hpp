//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include "MsixErrors.hpp"

#include "msixtest.hpp"

#include <string>
#include <map>

namespace MsixTest {

    // Singleton class that return the relative path of the test data
    class TestPath
    {
    public:
        typedef enum
        {
            Output,
            Unpack,
            Unbundle,
            Flat,
            BadFlat
        } Directory;

        static TestPath* GetInstance();
        void SetRoot(const char* root);
        std::string GetRoot();
        std::string GetPath(Directory opt);

    private:
        TestPath() {}
        TestPath(const TestPath&);
        TestPath& operator=(const TestPath&);

        static TestPath* m_instance;
        std::string m_root;
    };

    namespace Allocators
    {
        LPVOID STDMETHODCALLTYPE Allocate(SIZE_T cb);
        void STDMETHODCALLTYPE Free(LPVOID pv);
    }

    namespace Constants
    {
        namespace Package{
            // Package footprint files
            static const std::pair<APPX_FOOTPRINT_FILE_TYPE, std::string> AppxManifest =  
                { APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "AppxManifest.xml" };
            static const std::pair<APPX_FOOTPRINT_FILE_TYPE, std::string> AppxBlockMap =  
                { APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "AppxBlockMap.xml" };
            static const std::pair<APPX_FOOTPRINT_FILE_TYPE, std::string> AppxSignature =  
                { APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "AppxSignature.p7x" };
            static const std::pair<APPX_FOOTPRINT_FILE_TYPE, std::string> CodeIntegrity =  
                { APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "AppxMetadata/CodeIntegrity.cat" };
        }

        namespace Bundle
        {
            // Bundle footprint files
            static const std::pair<APPX_BUNDLE_FOOTPRINT_FILE_TYPE, std::string> AppxBundleManifest =  
                { APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST, "AppxMetadata/AppxBundleManifest.xml" };
            static const std::pair<APPX_BUNDLE_FOOTPRINT_FILE_TYPE, std::string> AppxBlockMap =  
                { APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP, "AppxBlockMap.xml" };
            static const std::pair<APPX_BUNDLE_FOOTPRINT_FILE_TYPE, std::string> AppxSignature =  
                { APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE, "AppxSignature.p7x" };
        }
    }

    namespace String
    {
        std::string utf16_to_utf8(const std::wstring& utf16string);
        std::wstring utf8_to_utf16(const std::string& utf8string);
    }

    // Helper class to free buffers obtained from the packaging APIs.
    namespace Wrappers
    {
        template<typename T>
        class Buffer
        {
        public:
            T** operator&() { return &content; }
            ~Buffer() { Cleanup(); }
            T* Get() { return content; }
            std::string ToString() { static_assert(False<T>::value, "ToString not supported"); };
        protected:
            T* content = nullptr;
            void Cleanup() { if (content) { Allocators::Free(content); content = nullptr; } }
        };

        template<>
        inline std::string Buffer<char>::ToString() { return std::string(content); }

        template<>
        inline std::string Buffer<wchar_t>::ToString() { return String::utf16_to_utf8(content); }
    }

    namespace Log
    {
        void PrintMsixLog(HRESULT actual, HRESULT result);
    }

    template <class T>
    class ComPtr
    {
    public:
        // default ctor
        ComPtr() = default;
        ComPtr(T* ptr) : m_ptr(ptr) { InternalAddRef(); }

        ~ComPtr() { InternalRelease(); }

        // For use instead of ComPtr<T> t(new Foo(...));
        template<class U, class... Args>
        static ComPtr<T> Make(Args&&... args)
        {
            ComPtr<T> result;
            result.m_ptr = new U(std::forward<Args>(args)...);
            return result;
        }
        
        inline T* operator->() const { return m_ptr; }
        inline T* Get() const { return m_ptr; }
        T* Detach() 
        {
            T* temp = m_ptr;
            m_ptr = nullptr;
            return temp;
        }

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
}
