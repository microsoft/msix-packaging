//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxBundleWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"
#include "FileNameValidation.hpp"
#include "StringHelper.hpp"

#include <string>
#include <memory>
#include <future>
#include <algorithm>
#include <functional>

#include <zlib.h>

namespace MSIX {

    AppxBundleWriter::AppxBundleWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip, std::uint64_t bundleVersion)
        : m_factory(factory), m_zipWriter(zip)
    {
        m_state = WriterState::Open;
        if(bundleVersion == 0)
        {
            SYSTEMTIME time = {0};
            GetSystemTime(&time);

            // The generated version number has the format: YYYY.MMDD.hhmm.0
            this->m_bundleWriterHelper.SetBundleVersion((static_cast<std::uint64_t>(time.wYear) << 48) |
                (static_cast<std::uint64_t>(time.wMonth * 100 + time.wDay) << 32) |
                (static_cast<std::uint64_t>(time.wHour * 100 + time.wMinute) << 16));
        }
        else
        {
            this->m_bundleWriterHelper.SetBundleVersion(bundleVersion);
        }
    }

    // IBundleWriter
    void AppxBundleWriter::ProcessBundlePayload(const ComPtr<IDirectoryObject>& from, bool flatBundle)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
            {
                this->m_state = WriterState::Failed;
            });

        auto fileMap = from->GetFilesByLastModDate();
        for (const auto& file : fileMap)
        {
            if (!(FileNameValidation::IsFootPrintFile(file.second, true)))
            {
                std::string ext = Helper::tolower(file.second.substr(file.second.find_last_of(".") + 1));
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                auto stream = from.As<IStorageObject>()->GetFile(file.second);

                if (flatBundle)
                {
                    ThrowHrIfFailed(AddPackageReference(utf8_to_wstring(file.second).c_str(), stream.Get(), 
                        false));
                }
            }
        }

        //Process external packages passed as input created from mapping file
        /*if (externalPackagesList != nullptr)
        {

        }*/

        failState.release();
    }

    // IAppxBundleWriter
    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddPayloadPackage(LPCWSTR fileName, IStream* packageStream) noexcept try
    {
        // TODO: implement
        NOTIMPLEMENTED;
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleWriter::Close() noexcept try
    {
        ThrowHrIfFailed(m_bundleWriterHelper.EndBundleManifest());

        auto bundleManifestStream = m_bundleWriterHelper.GetBundleManifestStream();
        auto bundleManifestContentType = ContentType::GetBundlePayloadFileContentType(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST);
        AddFileToPackage(APPXBUNDLEMANIFEST_XML, bundleManifestStream.Get(), true, true, bundleManifestContentType.c_str());

        CloseInternal();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxBundleWriter4
    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddPackageReference(LPCWSTR fileName, 
        IStream* inputStream, BOOL isDefaultApplicablePackage) noexcept try
    {   
        ThrowHrIfFailed(this->AddPackageReferenceInternal(fileName, inputStream, !!isDefaultApplicablePackage));

        return S_OK;

    } CATCH_RETURN();

    HRESULT AppxBundleWriter::AddPackageReferenceInternal(
        _In_ LPCWSTR fileName,
        _In_ IStream* packageStream,
        _In_ bool isDefaultApplicablePackage)
    {
        auto appxFactory = m_factory.As<IAppxFactory>();

        ComPtr<IAppxPackageReader> reader;
        ThrowHrIfFailed(appxFactory->CreatePackageReader(packageStream, &reader));

        std::uint64_t packageStreamSize = 0;
        ThrowHrIfFailed(this->m_bundleWriterHelper.GetStreamSize(packageStream, &packageStreamSize));
        
        ThrowHrIfFailed(this->m_bundleWriterHelper.AddPackage(wstring_to_utf8(fileName), reader.Get(), 0, packageStreamSize, isDefaultApplicablePackage));

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddPayloadPackage(LPCWSTR fileName, IStream* packageStream, 
        BOOL isDefaultApplicablePackage) noexcept try
    {
        // TODO: implement
        NOTIMPLEMENTED;
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddExternalPackageReference(LPCWSTR fileName,
        IStream* inputStream, BOOL isDefaultApplicablePackage) noexcept try
    {
        // TODO: implement
        NOTIMPLEMENTED;
    } CATCH_RETURN();

    void AppxBundleWriter::AddFileToPackage(const std::string& name, IStream* stream, bool toCompress,
        bool addToBlockMap, const char* contentType, bool forceContentTypeOverride)
    {
        std::string opcFileName;
        // Don't encode [Content Type].xml
        if (contentType != nullptr)
        {
            opcFileName = Encoding::EncodeFileName(name);
        }
        else
        {
            opcFileName = name;
        }
        auto fileInfo = m_zipWriter->PrepareToAddFile(opcFileName, toCompress);

        // Add content type to [Content Types].xml
        if (contentType != nullptr)
        {
            m_contentTypeWriter.AddContentType(name, contentType, forceContentTypeOverride);
        }

        // This might be called with external IStream implementations. Don't rely on internal implementation of FileStream
        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
        std::uint64_t uncompressedSize = static_cast<std::uint64_t>(end.QuadPart);

        // Add file to block map.
        if (addToBlockMap)
        {
            m_blockMapWriter.AddFile(name, uncompressedSize, fileInfo.first);
        }

        auto& zipFileStream = fileInfo.second;

        std::uint64_t bytesToRead = uncompressedSize;
        std::uint32_t crc = 0;
        while (bytesToRead > 0)
        {
            // Calculate the size of the next block to add
            std::uint32_t blockSize = (bytesToRead > DefaultBlockSize) ? DefaultBlockSize : static_cast<std::uint32_t>(bytesToRead);
            bytesToRead -= blockSize;

            // read block from stream
            std::vector<std::uint8_t> block;
            block.resize(blockSize);
            ULONG bytesRead;
            ThrowHrIfFailed(stream->Read(static_cast<void*>(block.data()), static_cast<ULONG>(blockSize), &bytesRead));
            ThrowErrorIfNot(Error::FileRead, (static_cast<ULONG>(blockSize) == bytesRead), "Read stream file failed");
            crc = crc32(crc, block.data(), static_cast<uInt>(block.size()));

            // Write block and compress if needed
            ULONG bytesWritten = 0;
            ThrowHrIfFailed(zipFileStream->Write(block.data(), static_cast<ULONG>(block.size()), &bytesWritten));

            // Add block to blockmap
            if (addToBlockMap)
            {
                m_blockMapWriter.AddBlock(block, bytesWritten, toCompress);
            }

        }

        if (toCompress)
        {
            // Put the stream termination on
            std::vector<std::uint8_t> buffer;
            ULONG bytesWritten = 0;
            ThrowHrIfFailed(zipFileStream->Write(buffer.data(), static_cast<ULONG>(buffer.size()), &bytesWritten));
        }

        // Close File element
        if (addToBlockMap)
        {
            m_blockMapWriter.CloseFile();
        }

        // This could be the compressed or uncompressed size
        auto streamSize = zipFileStream.As<IStreamInternal>()->GetSize();
        m_zipWriter->EndFile(crc, streamSize, uncompressedSize, true);
    }

    // Common close functionality for pack and bundle.
    // Adds AppxBlockMap.xml, [Content_Types].xml and closes the zip object.
    void AppxBundleWriter::CloseInternal()
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
            {
                this->m_state = WriterState::Failed;
            });

        // Close blockmap and add it to package
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        AddFileToPackage(APPXBLOCKMAP_XML, blockMapStream.Get(), true, false, blockMapContentType.c_str());

        // Close content types and add it to package
        m_contentTypeWriter.Close();
        auto contentTypeStream = m_contentTypeWriter.GetStream();
        AddFileToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), true, false, nullptr);

        m_zipWriter->Close();
        failState.release();
        m_state = WriterState::Closed;
    }

}
