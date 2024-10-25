//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"
#include "FileNameValidation.hpp"
#include "StringHelper.hpp"
#include "SignatureCreator.hpp"
#include "StreamHelper.hpp"

#include <string>
#include <memory>
#include <future>
#include <algorithm>
#include <functional>

namespace MSIX {

    AppxPackageWriter::AppxPackageWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip, bool enableFileHash) : m_factory(factory), m_zipWriter(zip)
    {
        if (enableFileHash)
        {
            m_blockMapWriter.EnableFileHash();
        }
        m_state = WriterState::Open;
    }

    AppxPackageWriter::AppxPackageWriter(IPackage* packageToSign, std::unique_ptr<SignatureAccumulator>&& accumulator) :
        m_signatureAccumulator(std::move(accumulator)), m_contentTypeWriter(packageToSign->GetUnderlyingStorageObject()->GetFile(CONTENT_TYPES_XML).Get())
    {
        m_factory = packageToSign->GetFactory();
        m_zipWriter = ComPtr<IZipWriter>::Make<ZipObjectWriter>(packageToSign->GetUnderlyingStorageObject().As<IZipObject>().Get());

        // Remove the files that are modified by signing
        m_zipWriter->RemoveFiles({ CONTENT_TYPES_XML, CODEINTEGRITY_CAT, APPXSIGNATURE_P7X });
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
            if(!(FileNameValidation::IsFootPrintFile(file.second, false) || FileNameValidation::IsReservedFolder(file.second)))
            {
                std::string ext = Helper::tolower(file.second.substr(file.second.find_last_of(".") + 1));
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                auto stream = from.As<IStorageObject>()->GetFile(file.second);
                ValidateAndAddPayloadFile(file.second, stream.Get(), contentType.GetCompressionOpt(), contentType.GetContentType().c_str());
            }
        }
        failState.release();
    }

    void AppxPackageWriter::Close(
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        const char* pass,
        IStream* privateKey)
    {
        bool signing = static_cast<bool>(m_signatureAccumulator);
        ThrowErrorIf(Error::InvalidParameter, signing && signingCertificate == nullptr, "Writer opened for signing needs a certificate");

        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });

        ComPtr<IStream> catalogStream;

        if (signing)
        {
            // Add content type for signature
            m_contentTypeWriter.AddContentType(APPXSIGNATURE_P7X, ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_SIGNATURE), true);

            // Add content type for the catalog file if it exists
            catalogStream = m_signatureAccumulator->GetCodeIntegrityStream(signingCertificateFormat, signingCertificate, privateKey);
            if (catalogStream)
            {
                m_contentTypeWriter.AddContentType(CODEINTEGRITY_CAT, ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY), true);
            }
        }

        // Close content types and add it to package
        m_contentTypeWriter.Close();
        auto contentTypeStream = m_contentTypeWriter.GetStream();
        AddFileToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), true, false, nullptr, false, false);

        if (signing)
        {
            // Add the catalog after the content types to preserve historical ordering
            if (catalogStream)
            {
                AddFileToPackage(CODEINTEGRITY_CAT, catalogStream.Get(), true, false, nullptr, false, false);
            }

            auto digestData = m_signatureAccumulator->GetSignatureObject(m_zipWriter.Get());
            auto signatureStream = SignatureCreator::Sign(digestData.Get(), signingCertificateFormat, signingCertificate, pass, privateKey);
            AddFileToPackage(APPXSIGNATURE_P7X, signatureStream.Get(), true, false, nullptr, false, false);
        }

        m_zipWriter->Close();

        // Ensure that the stream does not have any additional data hanging off the end
        ComPtr<IStream> zipStream = m_zipWriter.As<IZipObject>()->GetStream();
        ULARGE_INTEGER fileSize = { 0 };
        ThrowHrIfFailed(zipStream->Seek({ 0 }, StreamBase::Reference::CURRENT, &fileSize));
        ThrowHrIfFailed(zipStream->SetSize(fileSize));

        failState.release();
        m_state = WriterState::Closed;
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
        auto manifestObj = ComPtr<IAppxManifestReader>::Make<AppxManifestObject>(m_factory.Get(), manifestStream.Get());
        auto manifestContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_MANIFEST);
        AddFileToPackage(APPXMANIFEST_XML, manifestStream.Get(), true, true, manifestContentType.c_str());

        // Close blockmap and add it to package
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        AddFileToPackage(APPXBLOCKMAP_XML, blockMapStream.Get(), true, false, blockMapContentType.c_str());

        failState.release();

        // Merge with standalone signing path, with no signing information.
        Close(MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_UNKNOWN, nullptr, nullptr, nullptr);

        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriterUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
        {
            this->m_state = WriterState::Failed;
        });
        ValidateAndAddPayloadFile(fileName, inputStream, compressionOption, contentType);
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
            std::string contentType = wstring_to_utf8(payloadFiles[i].contentType);
            ValidateAndAddPayloadFile(fileName, payloadFiles[i].inputStream, payloadFiles[i].compressionOption, contentType.c_str());
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
            ValidateAndAddPayloadFile(payloadFiles[i].fileName, payloadFiles[i].inputStream, payloadFiles[i].compressionOption, payloadFiles[i].contentType);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxPackageWriter::ValidateAndAddPayloadFile(const std::string& name, IStream* stream,
        APPX_COMPRESSION_OPTION compressionOpt, const char* contentType)
    {
        ThrowErrorIfNot(Error::InvalidParameter, FileNameValidation::IsFileNameValid(name), "Invalid file name");
        ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsFootPrintFile(name, false), "Trying to add footprint file to package");
        ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsReservedFolder(name), "Trying to add file in reserved folder");
        ValidateCompressionOption(compressionOpt);
        AddFileToPackage(name, stream, compressionOpt != APPX_COMPRESSION_OPTION_NONE, true, contentType);
    }

    void AppxPackageWriter::AddFileToPackage(const std::string& name, IStream* stream, bool toCompress,
        bool addToBlockMap, const char* contentType, bool forceContentTypeOverride, bool forceDataDescriptor)
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

        std::unique_ptr<SignatureAccumulator::FileAccumulator> fileAccumulator;
        if (m_signatureAccumulator)
        {
            fileAccumulator = m_signatureAccumulator->GetFileAccumulator(name);
        }

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

            // Send data to file accumulator for signature creation
            if (fileAccumulator)
            {
                fileAccumulator->AccumulateRaw(block);
            }

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
        m_zipWriter->EndFile(crc, streamSize, uncompressedSize, forceDataDescriptor);

        // Send entire zip stream to accumulator
        if (fileAccumulator)
        {
            // We have to ensure that we reset the output stream position
            ComPtr<IZipObject> zipObj = m_zipWriter.As<IZipObject>();
            Helper::StreamPositionReset positionReset{ zipObj->GetStream().Get() };
            fileAccumulator->AccumulateZip(zipObj->GetEntireZipFileStream(opcFileName).Get());
        }
    }

    void AppxPackageWriter::ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt)
    {
        bool result = ((compressionOpt == APPX_COMPRESSION_OPTION_NONE) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_NORMAL) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_MAXIMUM) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_FAST) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_SUPERFAST));
        ThrowErrorIfNot(Error::InvalidParameter, result, "Invalid compression option.");
    }

}
