//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxFactory.hpp"
#include "UnicodeConversion.hpp"
#include "Exceptions.hpp"
#include "AppxPackageObject.hpp"
#include "MSIXResource.hpp"
#include "MemoryStream.hpp"
#include "MsixFeatureSelector.hpp"
#include "AppxPackageWriter.hpp"
#include "AppxBundleWriter.hpp"
#include "ZipObjectWriter.hpp"

#ifdef BUNDLE_SUPPORT
#include "AppxBundleManifest.hpp"
#endif

namespace MSIX {
    // IAppxFactory
    HRESULT STDMETHODCALLTYPE AppxFactory::CreatePackageWriter (
        IStream* outputStream,
        APPX_PACKAGE_SETTINGS* ,//settings, TODO: plumb this through
        IAppxPackageWriter** packageWriter) noexcept try
    {
        THROW_IF_PACK_NOT_ENABLED
        ThrowErrorIf(Error::InvalidParameter, (outputStream == nullptr || packageWriter == nullptr || *packageWriter != nullptr), "Invalid parameter");
        // We should never be here is packing if disabled, but the compiler
        // is not smart enough to remove it and the linker will fail.
        #ifdef MSIX_PACK
        auto zip = ComPtr<IZipWriter>::Make<ZipObjectWriter>(outputStream);
        bool enableFileHash = m_factoryOptions & MSIX_FACTORY_OPTION_WRITER_ENABLE_FILE_HASH;
        auto result = ComPtr<IAppxPackageWriter>::Make<AppxPackageWriter>(this, zip, enableFileHash);
        *packageWriter = result.Detach();
        #endif
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreatePackageReader (
        IStream* inputStream,
        IAppxPackageReader** packageReader) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageReader == nullptr || *packageReader != nullptr), "Invalid parameter");
        auto zip = ComPtr<IStorageObject>::Make<ZipObject>(inputStream);
        auto result = ComPtr<IAppxPackageReader>::Make<AppxPackageObject>(this, m_validationOptions, m_applicabilityFlags, zip);
        *packageReader = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateManifestReader(
        IStream* inputStream,
        IAppxManifestReader** manifestReader) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (manifestReader == nullptr || *manifestReader != nullptr), "Invalid parameter");
        ComPtr<IStream> input(inputStream);
        auto result = ComPtr<IAppxManifestReader>::Make<AppxManifestObject>(this, input);
        *manifestReader = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateBlockMapReader (
        IStream* inputStream,
        IAppxBlockMapReader** blockMapReader) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (
            inputStream == nullptr || 
            blockMapReader == nullptr || 
            *blockMapReader != nullptr
        ),"bad pointer.");

        ComPtr<IStream> stream(inputStream);
        *blockMapReader = ComPtr<IAppxBlockMapReader>::Make<AppxBlockMapObject>(this, stream).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateValidatedBlockMapReader (
        IStream* inputStream,
        LPCWSTR signatureFileName,
        IAppxBlockMapReader** blockMapReader) noexcept try
    {
        return CreateValidatedBlockMapReader(inputStream, wstring_to_utf8(signatureFileName).c_str(), blockMapReader);
    } CATCH_RETURN();

    // IAppxBundleFactory
    HRESULT STDMETHODCALLTYPE AppxFactory::CreateBundleWriter(IStream *outputStream, UINT64 bundleVersion, IAppxBundleWriter **bundleWriter) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        ThrowErrorIf(Error::InvalidParameter, (outputStream == nullptr || bundleWriter == nullptr || *bundleWriter != nullptr), "Invalid parameter");
        #ifdef MSIX_PACK 
        ComPtr<IMsixFactory> self;
        ThrowHrIfFailed(QueryInterface(UuidOfImpl<IMsixFactory>::iid, reinterpret_cast<void**>(&self)));
        auto zip = ComPtr<IZipWriter>::Make<ZipObjectWriter>(outputStream);
        auto result = ComPtr<IAppxBundleWriter>::Make<AppxBundleWriter>(self.Get(), zip, bundleVersion);
        *bundleWriter = result.Detach();
        #endif
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateBundleReader(IStream *inputStream, IAppxBundleReader **bundleReader) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        ComPtr<IAppxPackageReader> reader;
        ThrowHrIfFailed(CreatePackageReader(inputStream, &reader));
        auto result = reader.As<IAppxBundleReader>();
        *bundleReader = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::CreateBundleManifestReader(IStream *inputStream, IAppxBundleManifestReader **manifestReader) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        ThrowErrorIf(Error::InvalidParameter, (manifestReader == nullptr || *manifestReader != nullptr), "Invalid parameter");
        #ifdef BUNDLE_SUPPORT
        ComPtr<IStream> input(inputStream);
        auto result = ComPtr<IAppxBundleManifestReader>::Make<AppxBundleManifestObject>(this, input);
        *manifestReader = result.Detach();
        #endif
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IMsixFactory
    HRESULT AppxFactory::MarshalOutString(std::string& internal, LPWSTR *result) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (result == nullptr || *result != nullptr), "bad pointer" );
        *result = nullptr;
        if (!internal.empty())
        {
            auto intermediate = utf8_to_wstring(internal);
            ThrowHrIfFailed(MarshalOutWstring(intermediate, result));
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT AppxFactory::MarshalOutWstring(std::wstring& internal, LPWSTR* result) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (result == nullptr || *result != nullptr), "bad pointer" );
        *result = nullptr;
        if (!internal.empty())
        {
            std::size_t countBytes = sizeof(wchar_t)*(internal.size()+1);
            MarshalOutStringHelper<wchar_t>(countBytes, const_cast<wchar_t*>(internal.c_str()), result);
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT AppxFactory::MarshalOutStringUtf8(std::string& internal, LPSTR* result) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (result == nullptr || *result != nullptr), "bad pointer" );
        *result = nullptr;
        if (!internal.empty())
        {
            std::size_t countBytes = internal.size() + 1;
            MarshalOutStringHelper<char>(countBytes, const_cast<char*>(internal.c_str()), result);
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT AppxFactory::MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (size==nullptr || buffer == nullptr || *buffer != nullptr), "Bad pointer");
        *size = static_cast<UINT32>(data.size());
        *buffer = reinterpret_cast<BYTE*>(m_memalloc(data.size()));
        ThrowErrorIfNot(Error::OutOfMemory, (*buffer), "Allocation failed");
        std::memcpy(reinterpret_cast<void*>(*buffer),
                    reinterpret_cast<void*>(data.data()),
                    data.size());
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    ComPtr<IStream> AppxFactory::GetResource(const std::string& resource)
    {
        // Short-circuit the case where there were no resources and throw not found immediately.
        if (Resource::resourceLength <= 1)
        {
            ThrowErrorAndLog(Error::FileNotFound, resource.c_str());
        }

        if(!m_resourcezip) // Initialize it when first needed.
        {
            // Get stream of the resource zip file generated at CMake processing.
            m_resourcesVector = std::vector<std::uint8_t>(Resource::resourceByte, Resource::resourceByte + Resource::resourceLength);
            auto resourceStream = ComPtr<IStream>::Make<MemoryStream>(&m_resourcesVector);
            m_resourcezip = ComPtr<IStorageObject>::Make<ZipObject>(resourceStream.Get());
        }
        auto file = m_resourcezip->GetFile(resource);
        ThrowErrorIfNot(Error::FileNotFound, file, resource.c_str());
        return file;
    }

    // IMsixFactoryOverrides
    HRESULT STDMETHODCALLTYPE AppxFactory::SpecifyExtension(MSIX_FACTORY_EXTENSION name, IUnknown* extension) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (extension == nullptr), "Invalid parameter");

        if (name == MSIX_FACTORY_EXTENSION_STREAM_FACTORY)
        {
            ThrowHrIfFailed(extension->QueryInterface(UuidOfImpl<IMsixStreamFactory>::iid, reinterpret_cast<void**>(&m_streamFactory)));
        }
        else if (name == MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES)
        {
            ThrowHrIfFailed(extension->QueryInterface(UuidOfImpl<IMsixApplicabilityLanguagesEnumerator>::iid, reinterpret_cast<void**>(&m_applicabilityLanguagesEnumerator)));
        }
        else
        {
            return static_cast<HRESULT>(Error::InvalidParameter);
        }

        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxFactory::GetCurrentSpecifiedExtension(MSIX_FACTORY_EXTENSION name, IUnknown** extension) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (extension == nullptr || *extension != nullptr), "Invalid parameter");

        if (name == MSIX_FACTORY_EXTENSION_STREAM_FACTORY)
        {
            if (m_streamFactory.Get() != nullptr)
            {
                *extension = m_streamFactory.As<IUnknown>().Detach();
            }
        }
        else if (name == MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES)
        {
            if (m_applicabilityLanguagesEnumerator.Get() != nullptr)
            {
                *extension = m_applicabilityLanguagesEnumerator.As<IUnknown>().Detach();
            }
        }
        else
        {
            return static_cast<HRESULT>(Error::InvalidParameter);
        }

        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxFactoryUtf8
    HRESULT STDMETHODCALLTYPE AppxFactory::CreateValidatedBlockMapReader (
        IStream* inputStream,
        LPCSTR signatureFileName,
        IAppxBlockMapReader** blockMapReader) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (
            inputStream == nullptr || 
            signatureFileName == nullptr ||
            *signatureFileName == '\0' ||
            blockMapReader == nullptr || 
            *blockMapReader != nullptr
        ),"bad pointer.");

        auto stream = ComPtr<IStream>::Make<FileStream>(signatureFileName, FileStream::Mode::READ);
        auto signature = ComPtr<IVerifierObject>::Make<AppxSignatureObject>(this, this->GetValidationOptions(), stream);
        ComPtr<IStream> input(inputStream);
        auto validatedStream = signature->GetValidationStream("AppxBlockMap.xml", input);
        *blockMapReader = ComPtr<IAppxBlockMapReader>::Make<AppxBlockMapObject>(this, validatedStream).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // Helper to marshal out strings
    template<typename T>
    void AppxFactory::MarshalOutStringHelper(std::size_t size, T* from, T** to)
    {
        *to = reinterpret_cast<T*>(m_memalloc(size));
        ThrowErrorIfNot(Error::OutOfMemory, (*to), "Allocation failed!");
        std::memset(reinterpret_cast<void*>(*to), 0, size);
        std::memcpy(reinterpret_cast<void*>(*to),
                    reinterpret_cast<void*>(from),
                    size - sizeof(T));
    }

} // namespace MSIX 
