//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"

#include <string>
#include <memory>

namespace MSIX {

    static const std::uint32_t defaultBlockSize = 65536;

    AppxPackageWriter::AppxPackageWriter(IStream* outputStream) : m_outputStream(outputStream)
    {
        m_state = WriterState::Open;
        m_blockMapWriter = std::make_unique<BlockMapWriter>();
        m_contentTypeWriter = std::make_unique<ContentTypeWriter>();
    }

    // IPackageWriter
    void AppxPackageWriter::Pack(const ComPtr<IDirectoryObject>& from)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto fileMap = from->GetFilesByLastModDate();

        for(const auto& file : fileMap)
        {
            std::string ext = file.second.substr(file.second.find_last_of(".") + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            std::string contentType;
            APPX_COMPRESSION_OPTION compressionOpt;
            auto findExt = extToContentType.find(ext);
            if (findExt == extToContentType.end())
            {
                // if it's not in the map these are the defaults
                contentType = "application/octet-stream";
                compressionOpt = APPX_COMPRESSION_OPTION_NORMAL;
            }
            else
            {
                contentType = findExt->second.first;
                compressionOpt = findExt->second.second;
            }
            auto payloadFile = BuildPayloadFile(file.second, from.As<IStorageObject>()->GetFile(file.second), contentType, compressionOpt);
        }

        NOTIMPLEMENTED
    }

    // IAppxPackageWriter
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCWSTR fileName, LPCWSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream *inputStream) noexcept try
    {
        return AddPayloadFile(wstring_to_utf8(fileName).c_str(), wstring_to_utf8(contentType).c_str(), 
            compressionOption, inputStream);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageWriter::Close(IStream *manifest) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        // TODO: implement
        m_state = WriterState::Closed;
        NOTIMPLEMENTED
    } CATCH_RETURN();

    // IAppxPackageWriterUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        ComPtr<IStream> stream(inputStream);
        auto payloadFile = BuildPayloadFile(fileName, stream, contentType, compressionOption);
        // TODO: implement
        NOTIMPLEMENTED
    } CATCH_RETURN();

    // IAppxPackageWriter3
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        // TODO: handle parallelism
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        for(UINT32 i = 0; i < fileCount; i++)
        {
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            std::string fileName = wstring_to_utf8(payloadFiles[i].fileName);
            std::string contentType = wstring_to_utf8(payloadFiles[i].contentType);
            auto payloadFile = BuildPayloadFile(fileName, stream, contentType, payloadFiles[i].compressionOption);
        }
        
        NOTIMPLEMENTED
    } CATCH_RETURN();

    // IAppxPackageWriter3Utf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        for(UINT32 i = 0; i < fileCount; i++)
        {
            ComPtr<IStream> stream(payloadFiles[i].inputStream);
            auto payloadFile = BuildPayloadFile(payloadFiles[i].fileName, stream, payloadFiles[i].contentType, payloadFiles[i].compressionOption);
        }
        
        NOTIMPLEMENTED
    } CATCH_RETURN();

    std::unique_ptr<PayloadFile> AppxPackageWriter::BuildPayloadFile(const std::string& name, const ComPtr<IStream>& stream,
        const std::string& contentType, APPX_COMPRESSION_OPTION compressionOpt)
    {
        // This might be called with external IStream implementations. Don't rely on internal implementation of FileStream
        std::unique_ptr<PayloadFile> payloadFile = std::make_unique<PayloadFile>();
        payloadFile->relativeName = name;
        payloadFile->compressionOption = compressionOpt;

        // Add content type to [Content Types].xml
        m_contentTypeWriter->AddContentType(name, contentType);

        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
        payloadFile->fileSize = static_cast<std::uint64_t>(end.u.LowPart);

        std::unique_ptr<BlockAndHash> blockData;
        std::uint64_t bytesToRead = payloadFile->fileSize;

        while (bytesToRead > 0)
        {
            // Calculate the size of the next block to add
            std::uint32_t blockSize = (bytesToRead > defaultBlockSize) ? defaultBlockSize : static_cast<std::uint32_t>(bytesToRead);
            bytesToRead -= blockSize;

            std::vector<std::uint8_t> buffer;
            buffer.resize(blockSize);
            ULONG bytesRead;
            ThrowHrIfFailed(stream->Read(static_cast<void*>(buffer.data()), static_cast<ULONG>(blockSize), &bytesRead));
            ThrowErrorIfNot(Error::FileRead, (static_cast<ULONG>(blockSize) == bytesRead), "Read stream file failed");

            blockData.reset(new (std::nothrow) BlockAndHash());
            blockData->block = std::move(buffer);
            payloadFile->fileBlocks.push_back(std::move(blockData));
        }

        return payloadFile;
    }
}