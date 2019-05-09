//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include <iostream>
#include <atomic>
#include <vector>
#include <algorithm>
#include <locale>
#include <codecvt>

#ifndef WIN32
    // required posix-specific headers
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "Helpers.hpp"

using namespace MsixSample::Helper;

int Help()
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "\t" << "OverrideStreamSample.cpp -p <flat bundle> -rp <relative path of payload packages> -d <output directory>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tSample to show the usage of IMsixFactoryOverrides for MSIX_FACTORY_EXTENSION_STREAM_FACTORY " << std::endl;
    std::cout << "\twhere the directory of where the flat bundle is located doesn't contain the payload packages." << std::endl;
    std::cout << "\tUse -rp to specify the relative path for the location of the packages in the bundle." << std::endl;
    std::cout << std::endl;
    return 0;
}

// Hand rolled implementation of IStream with the minimum required to work
class MyStream final : public IStream
{
public:
    enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };

    // These are the same values as STREAM_SEEK. See
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa380359(v=vs.85).aspx for additional details.
    enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

    MyStream() : m_ref(1) {}
    ~MyStream()
    {
        if (m_file)
        {   // the most we would ever do w.r.t. a failure from fclose is *maybe* log something...
            std::fclose(m_file);
            m_file = nullptr;
        }
    }

    HRESULT Initialize(const std::string& name, Mode mode)
    {
        static const char* modes[] = { "rb", "wb", "ab", "r+b", "w+b", "a+b" };
        #ifdef WIN32
        errno_t err = fopen_s(&m_file, name.c_str(), modes[mode]);
        if (err != 0)
        {
            return E_INVALIDARG;
        }
        #else
        m_file = std::fopen(name.c_str(), modes[mode]);
        if (!m_file)
        {
            return E_INVALIDARG;
        }
        #endif

        // Get size of the file
        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };

        RETURN_IF_FAILED(Seek(start, Reference::END, &end));
        RETURN_IF_FAILED(Seek(start, Reference::START, nullptr));
        m_size = end.u.LowPart;
        return S_OK;
    }

    // IUnknown.
    // This is the loong way. Look at ComClass<> in src\inc\ComHelper.hpp for an example on how to avoid implementing IUnknown with pain.
    virtual ULONG STDMETHODCALLTYPE AddRef() noexcept override
    {
        return ++m_ref;
    }

    virtual ULONG STDMETHODCALLTYPE Release() noexcept override
    {
        if (--m_ref == 0)
        {
            delete this;
            return 0;
        }
        return m_ref;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override
    {
        if (ppvObject == nullptr || *ppvObject != nullptr)
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;
        if (riid == UuidOfImpl<IUnknown>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IUnknown*>(this));
            AddRef();
            return S_OK;
        }
        if (riid == UuidOfImpl<IStream>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IStream*>(this));
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    // IStream
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* newPosition) noexcept override
    {
        if (!m_file)
        {
            return E_INVALIDARG;
        }

        int rc = std::fseek(m_file, static_cast<long>(move.QuadPart), origin);
        if (rc != 0)
        {
            return E_INVALIDARG;
        }
        m_offset = Ftell();
        if (newPosition)
        {
            newPosition->QuadPart = m_offset;
        }
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override
    {
        if (!m_file)
        {
            return E_INVALIDARG;
        }

        if (bytesRead)
        {
            *bytesRead = 0;
        }
        ULONG result = static_cast<ULONG>(std::fread(buffer, sizeof(std::uint8_t), countBytes, m_file));
        if ((result != countBytes) && !Feof())
        {
            return E_INVALIDARG;
        }
        m_offset = Ftell();
        if (bytesRead)
        {
            *bytesRead = result;
        }
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *stream, ULARGE_INTEGER bytesCount, ULARGE_INTEGER *bytesRead, ULARGE_INTEGER *bytesWritten) noexcept override
    {
        if (bytesRead) { bytesRead->QuadPart = 0; }
        if (bytesWritten) { bytesWritten->QuadPart = 0; }
        if(stream == nullptr)
         {
            return E_INVALIDARG;
        }

        static const ULONGLONG size = 1024;
        std::vector<std::int8_t> bytes(size);
        std::int64_t read = 0;
        std::int64_t written = 0;
        ULONG length = 0;

        while (0 < bytesCount.QuadPart)
        {
            ULONGLONG chunk = std::min(bytesCount.QuadPart, static_cast<ULONGLONG>(size));
            RETURN_IF_FAILED(Read(reinterpret_cast<void*>(bytes.data()), (ULONG)chunk, &length));
            if (length == 0) { break; }
            read += length;

            ULONG offset = 0;
            while (0 < length)
            {
                ULONG copy = 0;
                RETURN_IF_FAILED(stream->Write(reinterpret_cast<void*>(&bytes[offset]), length, &copy));
                offset += copy;
                written += copy;
                length -= copy;
                bytesCount.QuadPart -= copy;
            }
        }

        if (bytesRead)      { bytesRead->QuadPart = read; }
        if (bytesWritten)   { bytesWritten->QuadPart = written;}
        return S_OK;
    }

    // For more information see: https://docs.microsoft.com/en-us/windows/desktop/api/objidl/nn-objidl-istream
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE Revert() noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* , DWORD) noexcept override { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream**) noexcept override { return E_NOTIMPL; }

    virtual HRESULT STDMETHODCALLTYPE Write(const void*, ULONG, ULONG*) noexcept override { return E_NOTIMPL; }

protected:
    inline int Ferror() { return std::ferror(m_file); }
    inline bool Feof()  { return 0 != std::feof(m_file); }
    inline void Flush() { std::fflush(m_file); }
    inline std::uint64_t Ftell()
    {
        auto result = std::ftell(m_file);
        return static_cast<std::uint64_t>(result);
    }

    std::uint64_t m_offset = 0;
    std::uint64_t m_size = 0;
    FILE* m_file;
    std::atomic<std::uint32_t> m_ref;
};

class MyStreamFactory final : public IMsixStreamFactory
{
public:
    MyStreamFactory(const std::wstring& path) : m_path(path), m_ref(1) {}

    // IUnknown
    virtual ULONG STDMETHODCALLTYPE AddRef() noexcept override
    {
        return ++m_ref;
    }

    virtual ULONG STDMETHODCALLTYPE Release() noexcept override
    {
        if (--m_ref == 0)
        {
            delete this;
            return 0;
        }
        return m_ref;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override
    {
        if (ppvObject == nullptr || *ppvObject != nullptr)
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;
        if (riid == UuidOfImpl<IUnknown>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IUnknown*>(this));
            AddRef();
            return S_OK;
        }
        if (riid == UuidOfImpl<IMsixStreamFactory>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IMsixStreamFactory*>(this));
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    // IMsixStreamFactory
    virtual HRESULT STDMETHODCALLTYPE CreateStreamOnRelativePath(LPCWSTR relativePath, IStream** stream) noexcept override
    {
        return CreateStreamOnRelativePathUtf8(utf16_to_utf8(relativePath).c_str(), stream);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateStreamOnRelativePathUtf8(LPCSTR relativePath, IStream** stream) noexcept override
    {
        *stream =  nullptr;
        ComPtr<IStream> result;
        auto path = utf16_to_utf8(m_path);
        #ifdef WIN32
        std::string fullFileName = path + std::string("\\") + relativePath;
        #else
        std::string fullFileName = path + std::string("/") + relativePath;
        std::replace(fullFileName.begin(), fullFileName.end(), '\\', '/' );
        #endif
        RETURN_IF_FAILED(ComPtr<IStream>::MakeAndInitialize<MyStream>(&result, fullFileName, MyStream::Mode::READ));
        if (result.Get() != nullptr)
        {
            *stream = result.Detach();
        }
        return S_OK;
    }

protected:
    std::wstring m_path;
    std::atomic<std::uint32_t> m_ref;
};

HRESULT GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream)
{
    const int MaxFileNameLength = 200;
    #ifdef WIN32
    std::wstring fullFileName = path + std::wstring(L"\\") + fileName;
    #else
    std::wstring fullFileName = path + std::wstring(L"/") + fileName;
    std::replace(fullFileName.begin(), fullFileName.end(), '\\', '/' );
    #endif

    RETURN_IF_FAILED(HRESULT_FROM_WIN32(mkdirp(fullFileName)));
    // Create stream for writing the file
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(fullFileName.c_str(), false, stream));
    return S_OK;
}

HRESULT ExtractFile(IAppxFile* file, LPCWSTR outputPath)
{
    Text<WCHAR> fileName;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    // Get basic info about the file
    RETURN_IF_FAILED(file->GetName(&fileName));
    RETURN_IF_FAILED(file->GetSize(&fileSize));
    fileSizeLargeInteger.QuadPart = fileSize;

    std::printf("\tPackage name: %s\n" , utf16_to_utf8(fileName.Get()).c_str());
    std::printf("\tSize: %llu bytes\n\n", fileSize);

    // Write the file to disk
    RETURN_IF_FAILED(file->GetStream(&fileStream));
    RETURN_IF_FAILED(GetOutputStream(outputPath, fileName.Get(), &outputStream));
    RETURN_IF_FAILED(fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr));
    return S_OK;
}

HRESULT UnpackFlatBundle(const std::string& flatBundle, const std::wstring& relativePath, const std::wstring& outputPath)
{
    // Create factory
    ComPtr<IAppxBundleFactory> bundleFactory;
    RETURN_IF_FAILED(CoCreateAppxBundleFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL,
        &bundleFactory));

    // Create out object that implements IMsixStreamFactory
    auto streamFactory = ComPtr<IMsixStreamFactory>::Make<MyStreamFactory>(relativePath);

    // AppxPackaging.hpp contains a helper UuidOfImpl<I>::iid for QueryInterface. It returns the GUID associated with the interface.
    ComPtr<IUnknown> unk;
    RETURN_IF_FAILED(streamFactory->QueryInterface(UuidOfImpl<IUnknown>::iid, reinterpret_cast<void**>(&unk)));

    // Get the IMsixFactoryOverrides and set your override.
    ComPtr<IMsixFactoryOverrides> factoryOverrides;
    RETURN_IF_FAILED(bundleFactory->QueryInterface(UuidOfImpl<IMsixFactoryOverrides>::iid, reinterpret_cast<void**>(&factoryOverrides)));
    RETURN_IF_FAILED(factoryOverrides->SpecifyExtension(MSIX_FACTORY_EXTENSION_STREAM_FACTORY, unk.Get()));

    // Create the stream of the flat bundle and the bundle reader
    ComPtr<IStream> inputStream;
    RETURN_IF_FAILED(CreateStreamOnFile(const_cast<char*>(flatBundle.c_str()), true, &inputStream));

    ComPtr<IAppxBundleReader> bundleReader;
    RETURN_IF_FAILED(bundleFactory->CreateBundleReader(inputStream.Get(), &bundleReader));

    // For this sample, lets just unpack the payload packages
    ComPtr<IAppxFilesEnumerator> packages;
    std::printf("Extracting payload files from the package...\n");

    // Get an enumerator of all payload packages from the bundle reader and iterate through all files.
    RETURN_IF_FAILED(bundleReader->GetPayloadPackages(&packages));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(packages->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxFile> package;
        RETURN_IF_FAILED(packages->GetCurrent(&package));
        // See ExtractContentSample on how to unpack recursively the packages in a bundle
        RETURN_IF_FAILED(ExtractFile(package.Get(), outputPath.c_str()));
        RETURN_IF_FAILED(packages->MoveNext(&hasCurrent));
    }

    return S_OK;
}

int main(int argc, char* argv[])
{
    if (argc != 7)
    {
        return Help();
    }

    std::string flatBundle;
    std::wstring relativePath;
    std::wstring directory;
    int index = 1;
    while (index < argc)
    {
        if (strcmp(argv[index], "-p") == 0)
        {
            flatBundle = argv[++index];
            index++;
        }
        else if (strcmp(argv[index], "-rp") == 0)
        {
            relativePath = utf8_to_utf16(argv[++index]);
            index++;
        }
        else if (strcmp(argv[index], "-d") == 0)
        {
            directory = utf8_to_utf16(argv[++index]);
            index++;
        }
        else
        {
            return Help();
        }
    }

    if (flatBundle.empty() || relativePath.empty() || directory.empty())
    {
        return Help();
    }

    HRESULT hr = UnpackFlatBundle(flatBundle, relativePath, directory);

    if (FAILED(hr))
    {
        std::cout << "Error: " << std::hex << hr << " while extracting the appx package" <<std::endl;
        Text<char> text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {   std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {   std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }

    return 0;
}
