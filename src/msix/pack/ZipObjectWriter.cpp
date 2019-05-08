//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "ZipObjectWriter.hpp"
#include "ZipObject.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"

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
            Field<0>().value = static_cast<std::uint32_t>(Signatures::DataDescriptor);
            Field<1>().value = crc;
            Field<2>().value = compressSize;
            Field<3>().value = uncompressSize;
        }
    };

    ZipObjectWriter::ZipObjectWriter(const ComPtr<IStream>& stream) : ZipObject(stream)
    {
    }

    // This is used for editing a package (aka signing)
    ZipObjectWriter::ZipObjectWriter(const ComPtr<IStorageObject>& storageObject) : ZipObject(storageObject)
    {
        // The storage object provided should had already initialize all the data.
        ThrowErrorIfNot(Error::Zip64EOCDLocator, m_endCentralDirectoryRecord.GetArchiveHasZip64Locator(),
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
    std::uint32_t ZipObjectWriter::PrepareToAddFile(std::string& name, bool isCompressed)
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");
        // Get position were the lfh is going to be written
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        // write lfh
        LocalFileHeader lfh;
        lfh.SetData(name, isCompressed);
        lfh.WriteTo(m_stream);
        m_lastLFH = std::make_pair(static_cast<std::uint64_t>(pos.QuadPart), std::move(lfh));
        m_state = ZipObjectWriter::State::ReadyForFile;
        return static_cast<std::uint32_t>(m_lastLFH.second.Size());
    }

    void ZipObjectWriter::AddFile(MSIX::ComPtr<IStream>& fileStream, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize)
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForFile, "Invalid zip writer state");
        // Write file stream
        LARGE_INTEGER start = { 0 };
        ThrowHrIfFailed(fileStream->Seek(start, StreamBase::Reference::START, nullptr));
        ULARGE_INTEGER bytesCount = { 0 };
        bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
        ThrowHrIfFailed(fileStream->CopyTo(m_stream.Get(), bytesCount, nullptr, nullptr));
        // Create and write data descriptor 
        DataDescriptor descriptor = DataDescriptor(crc, compressedSize, uncompressedSize);
        descriptor.WriteTo(m_stream);
        // Create and add cdh to map
        CentralDirectoryFileHeader cdh;
        auto name = m_lastLFH.second.GetFileName();
        cdh.SetData(name, crc, compressedSize, uncompressedSize, m_lastLFH.first, m_lastLFH.second.GetCompressionMethod());
        m_centralDirectories.insert(std::make_pair(name, std::move(cdh)));
        m_state = ZipObjectWriter::State::ReadyForLfhOrClose;
    }

    void ZipObjectWriter::Close()
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");
        // Write central directories
        ULARGE_INTEGER startOfCdh = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &startOfCdh));
        std::size_t cdhsSize = 0;
        for (auto& cdh : m_centralDirectories)
        {
            cdhsSize += cdh.second.Size();
            cdh.second.WriteTo(m_stream);
        }

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