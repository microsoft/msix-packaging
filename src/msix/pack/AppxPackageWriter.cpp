//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "ZipFileStream.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"
#include "SignatureCreator.hpp"
#include "StreamHelper.hpp"

#include <string>
#include <memory>
#include <future>
#include <algorithm>
#include <functional>

#include <zlib.h>

namespace MSIX {

    AppxPackageWriter::AppxPackageWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip) : m_factory(factory), m_zipWriter(zip)
    {
    }

    AppxPackageWriter::AppxPackageWriter(IPackage* packageToSign, std::unique_ptr<SignatureAccumulator>&& accumulator, ContentTypeWriter&& writer) :
        m_signatureAccumulator(std::move(accumulator)), m_contentTypeWriter(std::move(writer))
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
            if(!IsFootPrintFile(file.second))
            {
                std::string ext = file.second.substr(file.second.find_last_of(".") + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                ProcessFileAndAddToPackage(file.second, from.As<IStorageObject>()->GetFile(file.second).Get(),
                    contentType.GetCompressionOpt(), contentType.GetContentType().c_str(), false);
            }
        }
        failState.release();
    }

    void AppxPackageWriter::Close(
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
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
        ProcessFileAndAddToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), APPX_COMPRESSION_OPTION_NORMAL,
            nullptr /*dont add to content type*/, false, false /*addToBlockMap*/);

        if (signing)
        {
            // Add the catalog after the content types to preserve historical ordering
            if (catalogStream)
            {
                ProcessFileAndAddToPackage(CODEINTEGRITY_CAT, catalogStream.Get(), APPX_COMPRESSION_OPTION_NORMAL, nullptr, false, false);
            }

            auto digestData = m_signatureAccumulator->GetSignatureObject(m_zipWriter.Get());
            auto signatureStream = SignatureCreator::Sign(digestData.Get(), signingCertificateFormat, signingCertificate, privateKey);
            ProcessFileAndAddToPackage(APPXSIGNATURE_P7X, signatureStream.Get(), APPX_COMPRESSION_OPTION_NORMAL, nullptr, false, false);
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
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_MANIFEST],
            manifestStream.Get(), APPX_COMPRESSION_OPTION_NORMAL, manifestContentType.c_str(), false);

        // Close blockmap and add it to package
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        ProcessFileAndAddToPackage(footprintFiles[APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP],
            blockMapStream.Get(), APPX_COMPRESSION_OPTION_NORMAL, blockMapContentType.c_str(), true /* forceOverride*/ ,false /*addToBlockMap*/ );

        failState.release();

        // Merge with standalone signing path, with no signing information.
        Close(MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_UNKNOWN, nullptr, nullptr);

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
        ProcessFileAndAddToPackage(fileName, inputStream, compressionOption, contentType, false);
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
            std::string contentType = wstring_to_utf8(payloadFiles[i].contentType);
            ProcessFileAndAddToPackage(fileName, payloadFiles[i].inputStream, payloadFiles[i].compressionOption, contentType.c_str(), false);
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
            ProcessFileAndAddToPackage(payloadFiles[i].fileName, payloadFiles[i].inputStream, payloadFiles[i].compressionOption, payloadFiles[i].contentType, false);
        }
        failState.release();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxPackageWriter::ProcessFileAndAddToPackage(const std::string& name, IStream* stream,
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
        auto streamSize =  zipFileStream.As<IStreamInternal>()->GetSize();
        m_zipWriter->AddFile(zipFileStream, crc, streamSize, uncompressedSize);

        // Send entire zip stream to accumulator
        if (fileAccumulator)
        {
            // We have to ensure that we reset the output stream position
            ComPtr<IZipObject> zipObj = m_zipWriter.As<IZipObject>();
            Helper::StreamPositionReset positionReset{ zipObj->GetStream().Get() };
            fileAccumulator->AccumulateZip(zipObj->GetEntireZipFileStream(opcFileName).Get());
        }
    }

    bool AppxPackageWriter::IsFootPrintFile(std::string normalized)
    {
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        return ((normalized == "appxmanifest.xml") ||
                (normalized == "appxsignature.p7x") ||
                (normalized == "appxblockmap.xml") ||
                (normalized == "[content_types].xml") ||
                (normalized.rfind("appxmetadata", 0) != std::string::npos) ||
                (normalized.rfind("microsoft.system.package.metadata", 0) != std::string::npos));
    }

}
