//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "ZipObject.hpp"

#include <vector>
#include <map>
#include <memory>

// {350dd671-0c40-4cd7-9a5b-27456d604bd0}
#ifndef WIN32
interface IZipWriter : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IZipWriter : public IUnknown
#endif
{
public:
    // Writes the lfh header to the stream and return the size of the header
    virtual std::uint32_t PrepareToAddFile(std::string& name, bool isCompressed) = 0;

    // Writes the stream provided to the zip file, writes data descriptor and adds an entry
    // to the central directories map
    virtual void AddFile(MSIX::ComPtr<IStream>& fileStream, std::uint32_t crc,
            std::uint64_t compressedSize, std::uint64_t uncompressedSize) = 0;

    // Ends zip file by writing the central directory records, zip64 locator,
    // zip64 end of central directory and the end of central directories.
    virtual void Close() = 0;
};
MSIX_INTERFACE(IZipWriter, 0x350dd671,0x0c40,0x4cd7,0x9a,0x5b,0x27,0x45,0x6d,0x60,0x4b,0xd0);

namespace MSIX {

    class ZipObjectWriter final : public ComClass<ZipObjectWriter, IStorageObject, IZipWriter>, ZipObject
    {
    public:
        ZipObjectWriter(const ComPtr<IStream>& stream);

        ZipObjectWriter(const ComPtr<IStorageObject>& storageObject);

        // IStorage methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override;
        std::string GetFileName() override { NOTIMPLEMENTED };

        // IZipWriter
        std::uint32_t PrepareToAddFile(std::string& name, bool isCompressed) override;
        void AddFile(MSIX::ComPtr<IStream>& fileStream, std::uint32_t crc,
            std::uint64_t compressedSize, std::uint64_t uncompressedSize) override;
        void Close() override;

    protected:
        enum class State
        {
            ReadyForLfhOrClose,
            ReadyForFile,
            Closed,
        };

        State m_state = State::ReadyForLfhOrClose;
        std::pair<std::uint64_t, LocalFileHeader> m_lastLFH;
    };
}