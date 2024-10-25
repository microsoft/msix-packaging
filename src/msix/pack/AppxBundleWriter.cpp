//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "AppxPackaging.hpp"
#include "AppxBundleWriter.hpp"
#include "AppxFactory.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ContentType.hpp"
#include "Encoding.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxManifestObject.hpp"
#include "ScopeExit.hpp"
#include "FileNameValidation.hpp"
#include "StringHelper.hpp"

#include <ctime>
#include <iomanip>

namespace MSIX {

    AppxBundleWriter::AppxBundleWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip, std::uint64_t bundleVersion)
        : m_factory(factory), m_zipWriter(zip)
    {
        m_state = WriterState::Open;
        if(bundleVersion == 0)
        {
            // The generated version number has the format: YYYY.MMDD.hhmm.0
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::gmtime(&t);
            std::stringstream ss;
            ss << std::put_time(&tm, "%Y.%m%d.%H%M.0");
            this->m_bundleWriterHelper.SetBundleVersion(ConvertVersionStringToUint64(ss.str()));
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
                    ThrowHrIfFailed(AddPackageReference(utf8_to_wstring(file.second).c_str(), stream.Get(), false));
                }
            }
        }

        failState.release();
    }

    void AppxBundleWriter::ProcessBundlePayloadFromMappingFile(std::map<std::string, std::string> fileList, bool flatBundle)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
            {
                this->m_state = WriterState::Failed;
            });

        std::map<std::string, std::string>::iterator fileListIterator;
        for (fileListIterator = fileList.begin(); fileListIterator != fileList.end(); fileListIterator++)
        {
            std::string inputPath = fileListIterator->second;
            std::string outputPath = fileListIterator->first;

            if (!(FileNameValidation::IsFootPrintFile(inputPath, true)))
            {
                std::string ext = Helper::tolower(inputPath.substr(inputPath.find_last_of(".") + 1));
                auto contentType = ContentType::GetContentTypeByExtension(ext);
                auto stream = ComPtr<IStream>::Make<FileStream>(inputPath, FileStream::Mode::READ);

                if (flatBundle)
                {
                    ThrowHrIfFailed(AddPackageReference(utf8_to_wstring(outputPath).c_str(), stream.Get(), false));
                }
            }
        }
        failState.release();
    }

    void AppxBundleWriter::ProcessExternalPackages(std::map<std::string, std::string> externalPackagesList)
    {
        std::map<std::string, std::string>::iterator externalPackagesIterator;
        for (externalPackagesIterator = externalPackagesList.begin(); externalPackagesIterator != externalPackagesList.end(); externalPackagesIterator++)
        {
            std::string inputPath = externalPackagesIterator->second;
            std::string outputPath = externalPackagesIterator->first;

            if (!(FileNameValidation::IsFootPrintFile(inputPath, true)))
            {
                auto inputStream = ComPtr<IStream>::Make<FileStream>(inputPath, FileStream::Mode::READ);
                ThrowHrIfFailed(AddExternalPackageReference(utf8_to_wstring(outputPath).c_str(), inputStream.Get(), false));
            }
        }
    }

    // IAppxBundleWriter
    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddPayloadPackage(LPCWSTR fileName, IStream* packageStream) noexcept try
    {
        // TODO: implement
        NOTIMPLEMENTED;
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleWriter::Close() noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto failState = MSIX::scope_exit([this]
            {
                this->m_state = WriterState::Failed;
            });

        //Process AppxBundleManifest.xml and add it to the bundle
        m_bundleWriterHelper.EndBundleManifest();

        auto bundleManifestStream = m_bundleWriterHelper.GetBundleManifestStream();
        auto bundleManifestContentType = ContentType::GetBundlePayloadFileContentType(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST);
        AddFileToPackage(APPXBUNDLEMANIFEST_XML, bundleManifestStream.Get(), true, true, bundleManifestContentType.c_str());

        // Close blockmap and add it to the bundle
        m_blockMapWriter.Close();
        auto blockMapStream = m_blockMapWriter.GetStream();
        auto blockMapContentType = ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP);
        AddFileToPackage(APPXBLOCKMAP_XML, blockMapStream.Get(), true, false, blockMapContentType.c_str());

        // Close content types and add it to the bundle
        m_contentTypeWriter.Close();
        auto contentTypeStream = m_contentTypeWriter.GetStream();
        AddFileToPackage(CONTENT_TYPES_XML, contentTypeStream.Get(), true, false, nullptr);

        m_zipWriter->Close();
        failState.release();
        m_state = WriterState::Closed;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxBundleWriter4
    HRESULT STDMETHODCALLTYPE AppxBundleWriter::AddPackageReference(LPCWSTR fileName, 
        IStream* inputStream, BOOL isDefaultApplicablePackage) noexcept try
    {   
        this->AddPackageReferenceInternal(wstring_to_utf8(fileName), inputStream, !!isDefaultApplicablePackage);
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxBundleWriter::AddPackageReferenceInternal(std::string fileName, IStream* packageStream,
        bool isDefaultApplicablePackage)
    {
        auto appxFactory = m_factory.As<IAppxFactory>();

        ComPtr<IAppxPackageReader> reader;
        ThrowHrIfFailed(appxFactory->CreatePackageReader(packageStream, &reader));

        std::uint64_t packageStreamSize = this->m_bundleWriterHelper.GetStreamSize(packageStream);
                
        this->m_bundleWriterHelper.AddPackage(fileName, reader.Get(), 0, packageStreamSize, isDefaultApplicablePackage);
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
        this->AddExternalPackageReferenceInternal(wstring_to_utf8(fileName), inputStream, !!isDefaultApplicablePackage);
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void AppxBundleWriter::AddExternalPackageReferenceInternal(std::string fileName, IStream* packageStream, bool isDefaultApplicablePackage)
    {
        auto appxFactory = m_factory.As<IAppxFactory>();

        ComPtr<IAppxManifestReader> manifestReader;
        HRESULT hr = appxFactory->CreateManifestReader(packageStream, &manifestReader);
        if(SUCCEEDED(hr))
        {
            this->m_bundleWriterHelper.AddExternalPackageReferenceFromManifest(fileName, manifestReader.Get(), isDefaultApplicablePackage);
            return;
        }

        ComPtr<IAppxPackageReader> packageReader;
        hr = appxFactory->CreatePackageReader(packageStream, &packageReader);
        if(SUCCEEDED(hr))
        {
            ComPtr<IAppxManifestReader> manifestReader;
            ThrowHrIfFailed(packageReader->GetManifest(&manifestReader));
            this->m_bundleWriterHelper.AddExternalPackageReferenceFromManifest(fileName, manifestReader.Get(), isDefaultApplicablePackage);
            return;
        }

        ThrowErrorAndLog(Error::InvalidData, "The data is invalid.");
    }

    void AppxBundleWriter::ValidateAndAddPayloadFile(const std::string& name, IStream* stream,
        APPX_COMPRESSION_OPTION compressionOpt, const char* contentType)
    {
        ThrowErrorIfNot(Error::InvalidParameter, FileNameValidation::IsFileNameValid(name), "Invalid file name");
        ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsFootPrintFile(name, false), "Trying to add footprint file to package");
        ThrowErrorIf(Error::InvalidParameter, FileNameValidation::IsReservedFolder(name), "Trying to add file in reserved folder");
        ValidateCompressionOption(compressionOpt);
        AddFileToPackage(name, stream, compressionOpt != APPX_COMPRESSION_OPTION_NONE, true, contentType);
    }

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

    void AppxBundleWriter::ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt)
    {
        bool result = ((compressionOpt == APPX_COMPRESSION_OPTION_NONE) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_NORMAL) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_MAXIMUM) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_FAST) ||
                       (compressionOpt == APPX_COMPRESSION_OPTION_SUPERFAST));
        ThrowErrorIfNot(Error::InvalidParameter, result, "Invalid compression option.");
    }
}
