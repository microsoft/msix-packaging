//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "Crypto.hpp"
#include "ZipFileStream.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"

#include <string>
#include <memory>
#include <future>
#include <algorithm>
#include <functional>

#include <zlib.h>

namespace MSIX {

    AppxPackageWriter::AppxPackageWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip) : m_factory(factory), m_zipWriter(zip)
    {
        m_state = WriterState::Open;
    }

    // IPackageWriter
    void AppxPackageWriter::PackPayloadFiles(const ComPtr<IDirectoryObject>& from)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });

        auto fileMap = from->GetFilesByLastModDate();
        for(const auto& file : fileMap)
        {
            // If any footprint file is present, ignore it. We only require the AppxManifest.xml
            // and any other will be ignored and a new one will be created for the package. 
            if(!IsFootPrintFile(file.second))
            {
                std::string ext = file.second.substr(file.second.find_last_of(".") + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                ProcessFileAndAddToPackage(file.second, from.As<IStorageObject>()->GetFile(file.second),
                    contentType.GetCompressionOpt(), contentType.GetContentType().c_str(), false);
            }
        }
        failState.release();
    }

    // IAppxPackageWriter
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCWSTR fileName, LPCWSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream *inputStream) noexcept try
    {
        return AddPayloadFile(wstring_to_utf8(fileName).c_str(), wstring_to_utf8(contentType).c_str(), 
            compressionOption, inputStream);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageWriter::Close(IStream* manifest) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });

        ComPtr<IStream> manifestStream(manifest);

        // Process AppxManifest.xml
        // If the creating the AppxManifestObject succeeds, then the stream is valid.
        auto manifest = ComPtr<IAppxManifestReader>::Make<AppxManifestObject>(m_factory.Get(), manifestStream.Get());
        auto manifestContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_MANIFEST);
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_MANIFEST],
            manifestStream, APPX_COMPRESSION_OPTION_NORMAL, manifestContentType.c_str(), false);

        // Close blockmap and add it to package
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP],
            blockMapStream, APPX_COMPRESSION_OPTION_NORMAL, blockMapContentType.c_str(), true /* forceOverride*/ ,false /*addToBlockMap*/ );

        // Close content types and add it to package
        m_contentTypeWriter.Close();
        auto contentTypeStream = m_contentTypeWriter.GetStream();
        ProcessFileAndAddToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), APPX_COMPRESSION_OPTION_NORMAL,
            nullptr /*dont add to content type*/, false, false /*addToBlockMap*/);

        m_zipWriter->Close();
        failState.release();
        m_state = WriterState::Closed;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriterUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        ThrowErrorIf(Error::InvalidParameter, IsFootPrintFile(fileName), "Trying to add footprint file to package");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        ComPtr<IStream> stream(inputStream);
        ProcessFileAndAddToPackage(fileName, stream, compressionOption, contentType, false);
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
            auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        // TODO: use memoryLimit for how many files are going to be added
        for(UINT32 i = 0; i < fileCount; i++)
        {
            std::string fileName = wstring_to_utf8(payloadFiles[i].fileName);
            ThrowErrorIf(Error::InvalidParameter, IsFootPrintFile(fileName), "Trying to add footprint file to package");
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            std::string contentType = wstring_to_utf8(payloadFiles[i].contentType);
            ProcessFileAndAddToPackage(fileName, stream, payloadFiles[i].compressionOption, contentType.c_str(), false);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3Utf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        // TODO: use memoryLimit for how many files are going to be added
        for(UINT32 i = 0; i < fileCount; i++)
        {
            ThrowErrorIf(Error::InvalidParameter, IsFootPrintFile(payloadFiles[i].fileName), "Trying to add footprint file to package");
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            ProcessFileAndAddToPackage(payloadFiles[i].fileName, stream, payloadFiles[i].compressionOption, payloadFiles[i].contentType, false);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxPackageWriter::ProcessFileAndAddToPackage(const std::string& name, const ComPtr<IStream>& stream,
        APPX_COMPRESSION_OPTION compressionOpt, const char* contentType, bool forceContentTypeOverride, bool addToBlockMap)
    {
        bool toCompress = (compressionOpt != APPX_COMPRESSION_OPTION_NONE );

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
        std::uint64_t uncompressedSize = static_cast<std::uint64_t>(end.u.LowPart);

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
        auto lfhSize = m_zipWriter->PrepareToAddFile(opcFileName, toCompress);

        // Add file to block map
        if (addToBlockMap)
        {
            m_blockMapWriter.AddFile(name, uncompressedSize, lfhSize);
        }

        auto zipFileStream = ComPtr<IStream>::Make<ZipFileStream>(opcFileName, toCompress);

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

            // hash block
            std::vector<std::uint8_t> hash;
            ThrowErrorIfNot(MSIX::Error::SignatureInvalid, 
                MSIX::SHA256::ComputeHash(block.data(), static_cast<uint32_t>(block.size()), hash), 
                "Invalid signature");

            // Add block to blockmap
            if (addToBlockMap)
            {
                m_blockMapWriter.AddBlock(hash, bytesWritten, toCompress);
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
        auto streamSize =  zipFileStream.As<IStreamInternal>()->GetSize();
        m_zipWriter->AddFile(zipFileStream, crc, streamSize, uncompressedSize);
    }

    bool AppxPackageWriter::IsFootPrintFile(std::string normalized)
    {
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        return ((normalized == "appxmanifest.xml") ||
                (normalized == "appxsignature.p7x") ||
                (normalized == "[content_types].xml") ||
                (normalized.rfind("appxmetadata", 0) != std::string::npos) ||
                (normalized.rfind("microsoft.system.package.metadata", 0) != std::string::npos));
    }

}
