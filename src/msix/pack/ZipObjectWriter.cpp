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

#include <utility>

namespace MSIX {

    ZipObjectWriter::ZipObjectWriter(IStream* stream)
    {
        m_zipObject = ComPtr<IZipObject>::Make<ZipObject>(stream, false);
    }

    // This is used for editing a package (aka signing)
    ZipObjectWriter::ZipObjectWriter(IZipObject* zipObject) : m_zipObject(zipObject)
    {
        // The storage object provided should had already initialize all the data.
        ThrowErrorIfNot(Error::Zip64EOCDRecord, m_zipObject->GetEndCentralDirectoryRecord().GetIsZip64(),
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
    std::pair<std::uint32_t, ComPtr<IStream>> ZipObjectWriter::PrepareToAddFile(const std::string& name, bool isCompressed)
    {
        auto zipStream = m_zipObject->GetStream();

        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "Invalid zip writer state");

        // Get position were the lfh is going to be written
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(zipStream->Seek({0}, StreamBase::Reference::CURRENT, &pos));

        // Write lfh
        LocalFileHeader lfh;
        lfh.SetData(name, isCompressed);
        lfh.WriteTo(zipStream);

        m_lastLFH = std::make_pair(static_cast<std::uint64_t>(pos.QuadPart), std::move(lfh));
        m_state = ZipObjectWriter::State::ReadyForFile;

        ComPtr<IStream> newZipStream = ComPtr<IStream>::Make<ZipFileStream>(name, isCompressed, zipStream.Get());
        if (isCompressed)
        {
            newZipStream = ComPtr<IStream>::Make<DeflateStream>(newZipStream);
        }

        return std::make_pair(static_cast<std::uint32_t>(m_lastLFH.second.Size()), std::move(newZipStream));
    }

    void ZipObjectWriter::EndFile(std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize, bool forceDataDescriptor)
    {
        auto zipStream = m_zipObject->GetStream();

        ThrowErrorIf(Error::InvalidState, m_state != ZipObjectWriter::State::ReadyForFile, "Invalid zip writer state");

        if (forceDataDescriptor ||
            compressedSize > MaxSizeToNotUseDataDescriptor ||
            uncompressedSize > MaxSizeToNotUseDataDescriptor)
        {
            // Create and write data descriptor 
            DataDescriptor descriptor = DataDescriptor(crc, compressedSize, uncompressedSize);
            descriptor.WriteTo(zipStream);
        }
        else
        {
            // The sizes can fit in the LFH, rewrite it with the new data
            Helper::StreamPositionReset resetAfterLFHWrite{ zipStream.Get() };

            LARGE_INTEGER lfhLocation;
            lfhLocation.QuadPart = static_cast<LONGLONG>(m_lastLFH.first);
            ThrowHrIfFailed(zipStream->Seek(lfhLocation, StreamBase::Reference::START, nullptr));

            // We cannot change the size of the LFH, ensure that we don't accidentally
            size_t currentSize = m_lastLFH.second.Size();
            m_lastLFH.second.SetData(crc, compressedSize, uncompressedSize);
            ThrowErrorIf(Error::Unexpected, currentSize != m_lastLFH.second.Size(), "Cannot change the LFH size when updating it");

            m_lastLFH.second.WriteTo(zipStream);
        }

        // Create and add cdh to map
        CentralDirectoryFileHeader cdh;
        cdh.SetData(m_lastLFH.second.GetFileName(), crc, compressedSize, uncompressedSize, m_lastLFH.first, m_lastLFH.second.GetCompressionMethod(), forceDataDescriptor);
        m_zipObject->GetCentralDirectories().emplace_back(std::make_pair(m_lastLFH.second.GetFileName(), std::move(cdh)));
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