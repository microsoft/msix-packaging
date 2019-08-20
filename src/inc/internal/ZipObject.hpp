//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

#include "ComHelper.hpp"
#include "StorageObject.hpp"
#include "ZipObjectComponents.hpp"

#include <map>
#include <string>
#include <vector>

// internal interface
// {986355bc-4e9c-413b-8b2b-72c9aa3a594d}
#ifndef WIN32
interface IZipObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IZipObject : public IUnknown
#endif
{
public:
    // TODO: Try to make these more functional rather than publicizing the internals
    virtual MSIX::ComPtr<IStream> GetStream() = 0;
    virtual MSIX::EndCentralDirectoryRecord& GetEndCentralDirectoryRecord() = 0;
    virtual MSIX::Zip64EndOfCentralDirectoryLocator& GetZip64Locator() = 0;
    virtual MSIX::Zip64EndOfCentralDirectoryRecord& GetZip64EndOfCentralDirectory() = 0;
    virtual std::vector<std::pair<std::string, MSIX::CentralDirectoryFileHeader>>& GetCentralDirectories() = 0;
    virtual MSIX::ComPtr<IStream> GetEntireZipFileStream(const std::string& fileName) = 0;
};
MSIX_INTERFACE(IZipObject, 0x986355bc, 0x4e9c, 0x413b, 0x8b, 0x2b, 0x72, 0xc9, 0xaa, 0x3a, 0x59, 0x4d);

namespace MSIX {
    // This represents a raw stream over a.zip file.
    class ZipObject final : public ComClass<ZipObject, IStorageObject, IZipObject>
    {
    public:
        ZipObject(IStream* stream, bool readStream = true);

        // IStorageObject methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override;
        std::string GetFileName() override;

        // IZipObject
        ComPtr<IStream> GetStream() override;
        MSIX::EndCentralDirectoryRecord& GetEndCentralDirectoryRecord() override;
        MSIX::Zip64EndOfCentralDirectoryLocator& GetZip64Locator() override;
        MSIX::Zip64EndOfCentralDirectoryRecord& GetZip64EndOfCentralDirectory() override;
        std::vector<std::pair<std::string, CentralDirectoryFileHeader>>& GetCentralDirectories() override;
        MSIX::ComPtr<IStream> GetEntireZipFileStream(const std::string& fileName) override;

    private:
        ComPtr<IStream> m_stream;
        EndCentralDirectoryRecord m_endCentralDirectoryRecord;
        Zip64EndOfCentralDirectoryLocator m_zip64Locator;
        Zip64EndOfCentralDirectoryRecord m_zip64EndOfCentralDirectory;
        std::vector<std::pair<std::string, CentralDirectoryFileHeader>> m_centralDirectories;
        std::map<std::string, ComPtr<IStream>> m_streams;
    };
}
