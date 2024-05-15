//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "ZipObjectWriter.hpp"
#include "ZipObject.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"
#include "ZipFileStream.hpp"
#include "DeflateStream.hpp"
#include "StreamHelper.hpp"
#include "Encoding.hpp"

namespace MSIX {

    // We only use this for writting. If we ever decide to validate it, it needs to move to 
    // ZipObject and ZipObjectReader must validate it
    class DataDescriptor final : public Meta::StructuredObject<
        Meta::Field4Bytes, // 0 - data descriptor header signature  4 bytes(0x08074b50)
        Meta::Field4Bytes, // 1 - crc -32                           4 bytes
        Meta::Field8Bytes, // 2 - compressed size                   8 bytes(zip64)
        Meta::Field8Bytes  // 3 - uncompressed size                 8 bytes(zip64)
    >
    {
    public:
        DataDescriptor(std::uint32_t crc, std::uint64_t compressSize, std::uint64_t uncompressSize)
        {
            Field<0>() = static_cast<std::uint32_t>(Signatures::DataDescriptor);
            Field<1>() = crc;
            Field<2>() = compressSize;
            Field<3>() = uncompressSize;
        }
    };

    ZipObjectWriter::ZipObjectWriter(const ComPtr<IStream>& stream) : ZipObject(stream)
    {
    }

    // This is used for editing a package (aka signing)
    ZipObjectWriter::ZipObjectWriter(const ComPtr<IStorageObject>& storageObject) : ZipObject(storageObject)
    {
        // The storage object provided should had already initialize all the data.
        ThrowErrorIfNot(Error::Zip64EOCDRecord, m_endCentralDirectoryRecord.GetIsZip64(),
            "Editing non zip64 packages not supported");

        // Move the stream at the start of central directory record so we can start overwritting.
        // Central directory data in already in m_centralDirectories.
        LARGE_INTEGER pos = {0};
        pos.QuadPart = m_zip64EndOfCentralDirectory.GetOffsetStartOfCD();
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
    }

    // IStorage
    std::vector<std::string> ZipObjectWriter::GetFileNames(FileNameOptions options)
    {
        // TODO: implement
        NOTIMPLEMENTED;
    }

    ComPtr<IStream> ZipObjectWriter::GetFile(const std::string& fileName)
    {
        // TODO: implement
        NOTIMPLEMENTED;
    }

    // IZipWriter
    std::pair<std::uint32_t, ComPtr<IStream>> ZipObjectWriter::PrepareToAddFile(const std::string& name, bool isCompressed)
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");

        auto result = m_centralDirectories.find(name);
        if (result != m_centralDirectories.end())
        {
            auto message = "Adding duplicated file " + Encoding::DecodeFileName(name) + "to package";
            ThrowErrorAndLog(Error::DuplicateFile, message.c_str());
        }

        // Get position were the lfh is going to be written
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));

        // track the sequence of file names to sort the central directory upon Close
        m_fileNameSequence.push_back(name);

        // Write lfh
        LocalFileHeader lfh;
        lfh.SetData(name, isCompressed);
        lfh.WriteTo(m_stream);

        m_lastLFH = std::make_pair(static_cast<std::uint64_t>(pos.QuadPart), std::move(lfh));
        m_state = ZipObjectWriter::State::ReadyForFile;

        ComPtr<IStream> zipStream = ComPtr<IStream>::Make<ZipFileStream>(name, isCompressed, m_stream.Get());
        if (isCompressed)
        {
            zipStream = ComPtr<IStream>::Make<DeflateStream>(zipStream);
        }

        return std::make_pair(static_cast<std::uint32_t>(m_lastLFH.second.Size()), std::move(zipStream));
    }

    void ZipObjectWriter::EndFile(std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize, bool forceDataDescriptor)
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForFile, "Invalid zip writer state");

        if (forceDataDescriptor ||
            compressedSize > MaxSizeToNotUseDataDescriptor ||
            uncompressedSize > MaxSizeToNotUseDataDescriptor)
        {
            // Create and write data descriptor 
            DataDescriptor descriptor = DataDescriptor(crc, compressedSize, uncompressedSize);
            descriptor.WriteTo(m_stream);
        }
        else
        {
            // The sizes can fit in the LFH, rewrite it with the new data
            Helper::StreamPositionReset resetAfterLFHWrite{ m_stream.Get() };

            LARGE_INTEGER lfhLocation;
            lfhLocation.QuadPart = static_cast<LONGLONG>(m_lastLFH.first);
            ThrowHrIfFailed(m_stream->Seek(lfhLocation, StreamBase::Reference::START, nullptr));

            // We cannot change the size of the LFH, ensure that we don't accidentally
            size_t currentSize = m_lastLFH.second.Size();
            m_lastLFH.second.SetData(crc, compressedSize, uncompressedSize);
            ThrowErrorIf(Error::Unexpected, currentSize != m_lastLFH.second.Size(), "Cannot change the LFH size when updating it");

            m_lastLFH.second.WriteTo(m_stream);
        }

        // Create and add cdh to map
        CentralDirectoryFileHeader cdh;
        cdh.SetData(m_lastLFH.second.GetFileName(), crc, compressedSize, uncompressedSize, m_lastLFH.first, m_lastLFH.second.GetCompressionMethod(), forceDataDescriptor);
        m_centralDirectories.insert(std::make_pair(m_lastLFH.second.GetFileName(), std::move(cdh)));
        m_state = ZipObjectWriter::State::ReadyForLfhOrClose;
    }

    void ZipObjectWriter::Close()
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");
        // Write central directories
        ULARGE_INTEGER startOfCdh = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &startOfCdh));
        std::size_t cdhsSize = 0;
        for (const auto& fileName : m_fileNameSequence)
        {
            auto it = m_centralDirectories.find(fileName);
            if (it != m_centralDirectories.end())
            {
                auto& cdh = it->second;
                cdhsSize += cdh.Size();
                cdh.WriteTo(m_stream);
            }
        }
        m_fileNameSequence.clear();

        // Write zip64 end of cds
        ULARGE_INTEGER startOfZip64EndOfCds = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &startOfZip64EndOfCds));
        m_zip64EndOfCentralDirectory.SetData(m_centralDirectories.size(), static_cast<std::uint64_t>(cdhsSize), 
            static_cast<std::uint64_t>(startOfCdh.QuadPart));
        m_zip64EndOfCentralDirectory.WriteTo(m_stream);

        // Write zip64 locator
        m_zip64Locator.SetData(static_cast<std::uint64_t>(startOfZip64EndOfCds.QuadPart));
        m_zip64Locator.WriteTo(m_stream);

        // Because we only use zip64, EndCentralDirectoryRecord never changes
        m_endCentralDirectoryRecord.WriteTo(m_stream);

        m_state = ZipObjectWriter::State::Closed;
    }

}