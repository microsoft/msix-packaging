//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "ZipObjectReader.hpp"
#include "ComHelper.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"

#include <vector>

namespace MSIX {

    ZipObjectReader::ZipObjectReader(const ComPtr<IStream>& stream) : ZipObject(stream)
    {
        LARGE_INTEGER pos = {0};
        pos.QuadPart = -1 * m_endCentralDirectoryRecord.Size();
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
        m_endCentralDirectoryRecord.Read(m_stream.Get());

        // find where the zip central directory exists.
        std::uint64_t offsetStartOfCD = 0;
        std::uint64_t totalNumberOfEntries = 0;
        if (!m_endCentralDirectoryRecord.GetIsZip64())
        {
            offsetStartOfCD = m_endCentralDirectoryRecord.GetStartOfCentralDirectory();
            totalNumberOfEntries = m_endCentralDirectoryRecord.GetNumberOfCentralDirectoryEntries();
        }
        else
        {   // Make sure that we have a zip64 end of central directory locator
            pos.QuadPart = -1*(m_endCentralDirectoryRecord.Size() + m_zip64Locator.Size());
            ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
            m_zip64Locator.Read(m_stream.Get());

            // now read the end of zip central directory record
            pos.QuadPart = m_zip64Locator.GetRelativeOffset();
            ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
            m_zip64EndOfCentralDirectory.Read(m_stream.Get());
            offsetStartOfCD = m_zip64EndOfCentralDirectory.GetOffsetStartOfCD();
            totalNumberOfEntries = m_zip64EndOfCentralDirectory.GetTotalNumberOfEntries();
        }

        // read the zip central directory
        pos.QuadPart = offsetStartOfCD;
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
        for (std::uint32_t index = 0; index < totalNumberOfEntries; index++)
        {
            auto centralFileHeader = CentralDirectoryFileHeader();
            centralFileHeader.Read(m_stream.Get(), m_endCentralDirectoryRecord.GetIsZip64());
            // TODO: ensure that there are no collisions on name!
            m_centralDirectories.insert(std::make_pair(centralFileHeader.GetFileName(), std::move(centralFileHeader)));
        }

        if (m_endCentralDirectoryRecord.GetIsZip64())
        {   // We should have no data between the end of the last central directory header and the start of the EoCD
            ULARGE_INTEGER uPos = {0};
            ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &uPos));
            ThrowErrorIfNot(Error::ZipHiddenData, (uPos.QuadPart == m_zip64Locator.GetRelativeOffset()), "hidden data unsupported");
        }
    }

    // IStoreageObject
        std::vector<std::string> ZipObjectReader::GetFileNames(FileNameOptions)
    {
        std::vector<std::string> result;
        std::for_each(m_centralDirectories.begin(), m_centralDirectories.end(), [&result](auto it)
        {
            result.push_back(it.first);
        });
        return result;
    }

    // ZipObjectReader::GetFile has cache semantics. If not found on m_streams, get the file from the central directories.
    // Not finding a file is non-fatal
    ComPtr<IStream> ZipObjectReader::GetFile(const std::string& fileName)
    {
        auto result = m_streams.find(fileName);
        if (result == m_streams.end())
        {
            auto centralFileHeader = m_centralDirectories.find(fileName);
            if(centralFileHeader == m_centralDirectories.end())
            {
                return ComPtr<IStream>();
            }
            LARGE_INTEGER pos = {0};
            pos.QuadPart = centralFileHeader->second.GetRelativeOffsetOfLocalHeader();
            ThrowHrIfFailed(m_stream->Seek(pos, MSIX::StreamBase::Reference::START, nullptr));
            LocalFileHeader lfh = LocalFileHeader();
            lfh.Read(m_stream.Get(), centralFileHeader->second);

            auto fileStream = ComPtr<IStream>::Make<ZipFileStream>(
                centralFileHeader->first,
                centralFileHeader->second.GetCompressionMethod() == CompressionType::Deflate,
                centralFileHeader->second.GetRelativeOffsetOfLocalHeader() + lfh.Size(),
                centralFileHeader->second.GetCompressedSize(),
                m_stream.Get()
            );

            if (centralFileHeader->second.GetCompressionMethod() == CompressionType::Deflate)
            {
                fileStream = ComPtr<IStream>::Make<InflateStream>(std::move(fileStream), centralFileHeader->second.GetUncompressedSize());
            }
            ComPtr<IStream> result(fileStream);
            m_streams.insert(std::make_pair(centralFileHeader->first, std::move(fileStream)));
            return result;
        }
        return result->second;
    }

    std::string ZipObjectReader::GetFileName()
    {
        return m_stream.As<IStreamInternal>()->GetName();
    }
}
