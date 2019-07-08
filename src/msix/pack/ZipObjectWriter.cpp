//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "ZipObjectWriter.hpp"
#include "ZipObject.hpp"
#include "MsixErrors.hpp"
#include "Exceptions.hpp"

#include <utility>

namespace MSIX {

    // We only use this for writting. If we ever decide to validate it, it needs to move to 
    // ZipObject must validate it
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

    ZipObjectWriter::ZipObjectWriter(IStream* stream)
    {
        m_zipObject = ComPtr<IZipObject>::Make<ZipObject>(stream, false);
    }

    // This is used for editing a package (aka signing)
    ZipObjectWriter::ZipObjectWriter(IZipObject* zipObject) : m_zipObject(zipObject)
    {
        // The storage object provided should had already initialize all the data.
        ThrowErrorIfNot(Error::Zip64EOCDLocator, m_zipObject->GetEndCentralDirectoryRecord().GetArchiveHasZip64Locator(),
            "Editing non zip64 packages not supported");

        // Move the stream at the start of central directory record so we can start overwritting.
        // Central directory data is already in m_centralDirectories.
        LARGE_INTEGER pos = {0};
        pos.QuadPart = m_zipObject->GetZip64EndOfCentralDirectory().GetOffsetStartOfCD();
        ThrowHrIfFailed(m_zipObject->GetStream()->Seek(pos, StreamBase::Reference::START, nullptr));
    }

    // IZipObject
    ComPtr<IStream> ZipObjectWriter::GetStream()
    {
        return m_zipObject->GetStream();
    }

    MSIX::EndCentralDirectoryRecord& ZipObjectWriter::GetEndCentralDirectoryRecord()
    {
        return m_zipObject->GetEndCentralDirectoryRecord();
    }

    MSIX::Zip64EndOfCentralDirectoryLocator& ZipObjectWriter::GetZip64Locator()
    {
        return m_zipObject->GetZip64Locator();
    }

    MSIX::Zip64EndOfCentralDirectoryRecord& ZipObjectWriter::GetZip64EndOfCentralDirectory()
    {
        return m_zipObject->GetZip64EndOfCentralDirectory();
    }

    std::vector<std::pair<std::string, CentralDirectoryFileHeader>>& ZipObjectWriter::GetCentralDirectories()
    {
        return m_zipObject->GetCentralDirectories();
    }

    MSIX::ComPtr<IStream> ZipObjectWriter::GetEntireZipFileStream(const std::string& fileName)
    {
        return m_zipObject->GetEntireZipFileStream(fileName);
    }

    // IZipWriter
    std::uint32_t ZipObjectWriter::PrepareToAddFile(std::string& name, bool isCompressed)
    {
        auto zipStream = m_zipObject->GetStream();

        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");
        // Get position were the lfh is going to be written
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(zipStream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        // write lfh
        LocalFileHeader lfh;
        lfh.SetData(name, isCompressed);
        lfh.WriteTo(zipStream);
        m_lastLFH = std::make_pair(static_cast<std::uint64_t>(pos.QuadPart), std::move(lfh));
        m_state = ZipObjectWriter::State::ReadyForFile;
        return static_cast<std::uint32_t>(m_lastLFH.second.Size());
    }

    void ZipObjectWriter::AddFile(MSIX::ComPtr<IStream>& fileStream, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize)
    {
        auto zipStream = m_zipObject->GetStream();

        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForFile, "Invalid zip writer state");
        // Write file stream
        LARGE_INTEGER start = { 0 };
        ThrowHrIfFailed(fileStream->Seek(start, StreamBase::Reference::START, nullptr));
        ULARGE_INTEGER bytesCount = { 0 };
        bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
        ThrowHrIfFailed(fileStream->CopyTo(zipStream.Get(), bytesCount, nullptr, nullptr));
        // Create and write data descriptor 
        DataDescriptor descriptor = DataDescriptor(crc, compressedSize, uncompressedSize);
        descriptor.WriteTo(zipStream);
        // Create and add cdh to map
        CentralDirectoryFileHeader cdh;
        auto name = m_lastLFH.second.GetFileName();
        cdh.SetData(name, crc, compressedSize, uncompressedSize, m_lastLFH.first, m_lastLFH.second.GetCompressionMethod());
        m_zipObject->GetCentralDirectories().emplace_back(std::make_pair(name, std::move(cdh)));
        m_state = ZipObjectWriter::State::ReadyForLfhOrClose;
    }

    void ZipObjectWriter::RemoveFiles(const std::vector<std::string>& files)
    {
        std::vector<size_t> centralDirectoryIndexes;

        // Search from the back to find all of the files, as they are most likely there
        for (size_t i = m_zipObject->GetCentralDirectories().size(); i > 0; --i)
        {
            const auto& cd = m_zipObject->GetCentralDirectories()[i - 1];
            auto itr = std::find(files.begin(), files.end(), cd.first);
            if (itr != files.end())
            {
                centralDirectoryIndexes.emplace_back(i - 1);

                // Early out if we find all of the files
                if (centralDirectoryIndexes.size() == files.size())
                {
                    break;
                }
            }
        }

        // None of the given files were found
        if (centralDirectoryIndexes.empty())
        {
            return;
        }

        // Ensure that all of the files are at the end of the stream,
        // at least until we want to support more adventurous editing.
        size_t minimumIndex = m_zipObject->GetCentralDirectories().size() - centralDirectoryIndexes.size();
        for (size_t i : centralDirectoryIndexes)
        {
            ThrowErrorIf(Error::NotSupported, i < minimumIndex, "Removing files from the middle of the archive is not supported");
        }

        // Now that we know we have a contiguous block of files at the end, we can safely remove them from the stream.
        // Do that by simply moving the stream to point to the start of the LFH of the file.
        CentralDirectoryFileHeader& cdToRemove = m_zipObject->GetCentralDirectories()[minimumIndex].second;
        LARGE_INTEGER pos = { 0 };
        pos.QuadPart = cdToRemove.GetRelativeOffsetOfLocalHeader();
        ThrowHrIfFailed(m_zipObject->GetStream()->Seek(pos, MSIX::StreamBase::Reference::START, nullptr));

        // Remove the CD entries at the end
        m_zipObject->GetCentralDirectories().resize(minimumIndex);
    }

    void ZipObjectWriter::WriteCentralDirectoryToStream(IStream* stream)
    {
        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");

        // Write central directories
        ULARGE_INTEGER offsetToStartOfCD = { 0 };
        ThrowHrIfFailed(m_zipObject->GetStream()->Seek({ 0 }, StreamBase::Reference::CURRENT, &offsetToStartOfCD));

        std::size_t cdhsSize = 0;
        for (auto& cdh : m_zipObject->GetCentralDirectories())
        {
            cdhsSize += cdh.second.Size();
            cdh.second.WriteTo(stream);
        }

        // Write zip64 end of cds
        ULARGE_INTEGER startOfZip64EndOfCds{};
        startOfZip64EndOfCds.QuadPart = offsetToStartOfCD.QuadPart + cdhsSize;
        m_zipObject->GetZip64EndOfCentralDirectory().SetData(m_zipObject->GetCentralDirectories().size(), static_cast<std::uint64_t>(cdhsSize),
            static_cast<std::uint64_t>(offsetToStartOfCD.QuadPart));
        m_zipObject->GetZip64EndOfCentralDirectory().WriteTo(stream);

        // Write zip64 locator
        m_zipObject->GetZip64Locator().SetData(static_cast<std::uint64_t>(startOfZip64EndOfCds.QuadPart));
        m_zipObject->GetZip64Locator().WriteTo(stream);

        // Because we only use zip64, EndCentralDirectoryRecord never changes
        m_zipObject->GetEndCentralDirectoryRecord().WriteTo(stream);
    }

    void ZipObjectWriter::Close()
    {
        WriteCentralDirectoryToStream(m_zipObject->GetStream().Get());
        m_state = ZipObjectWriter::State::Closed;
    }

}