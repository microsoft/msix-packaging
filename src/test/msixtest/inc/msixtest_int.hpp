//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include "MsixErrors.hpp"
#include "ComHelper.hpp"

#include "msixtest.hpp"
#include "macros.hpp"

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
            BadFlat,
            Pack,
            Sign,
            Manifest,
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

    // Initialize helpers
    void InitializePackageReader(const std::string& package, IAppxPackageReader** packageReader);
    void InitializePackageReader(IStream* stream, IAppxPackageReader** packageReader);
    void InitializeBundleReader(const std::string& package, IAppxBundleReader** bundleReader);
    void InitializeManifestReader(const std::string& manifest, IAppxManifestReader** manifestReader);

    // Use the product ComPtr; enables sharing without updating every qualified use.
    using MSIX::ComPtr;

    // Helper class that creates a stream from a given file name.
    // toRead - true if the file already exists, false to create it
    // toDelete - true if the file should be deleted when the this object
    // goes out of scope.
    class StreamFile
    {
    public:
        StreamFile() : m_toDelete(false) {}
        StreamFile(std::string fileName, bool toRead, bool toDelete = false);
        StreamFile(std::wstring fileName, bool toRead, bool toDelete = false);
        ~StreamFile() { Clean(); }

        void Initialize(std::string fileName, bool toRead, bool toDelete = false);
        void Initialize(std::wstring fileName, bool toRead, bool toDelete = false);

        inline IStream* operator->() const { return m_stream.Get(); }
        inline IStream* Get() const { return m_stream.Get(); }
        IStream* Detach()
        {
            m_toDelete = false;
            return m_stream.Detach();
        }

    protected:
        void InitializeStream(std::string fileName, bool toRead);
        void Clean();

        bool m_toDelete;
        std::string m_fileName;
        ComPtr<IStream> m_stream;
    };
}
