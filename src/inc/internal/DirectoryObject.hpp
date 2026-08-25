//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "ComHelper.hpp"
#include "FileStream.hpp"

// internal interface
// {1675f000-9b74-49bb-ba31-94ed7c435c28}
#ifndef WIN32
interface IDirectoryObject : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IDirectoryObject : public IUnknown
#endif
{
public:
    // Opens a stream to a file by name in the storage object. If the file does not exist and mode is read,
    // or read + update, then nullptr is returned.  If the file is opened with write and it does not exist, 
    // then the file is created and an empty stream to the file is handed back to the caller.
    virtual MSIX::ComPtr<IStream> OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) = 0;

    // Returns a multipmap sorted by last modified time. Use multimap in the unlikely case there are two files
    // with the same last modified time.
    virtual std::multimap<std::uint64_t, std::string> GetFilesByLastModDate() = 0;
};
MSIX_INTERFACE(IDirectoryObject, 0x1675f000,0x9b74,0x49bb,0xba,0x31,0x94,0xed,0x7c,0x43,0x5c,0x28);

namespace MSIX {

    class DirectoryObject final : public ComClass<DirectoryObject, IStorageObject, IDirectoryObject>
    {
    public:
        DirectoryObject(const std::string& root, bool createRootIfNecessary = false);

        // IStorageObject methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override
        {
            std::string file = m_root + GetPathSeparator() + fileName;
            auto fileStream = ComPtr<IStream>::Make<FileStream>(file, FileStream::Mode::READ);
            return fileStream;
        }
        std::string GetFileName() override { return m_root; }

        // IDirectoryObject
        ComPtr<IStream> OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) override;
        std::multimap<std::uint64_t, std::string> GetFilesByLastModDate() override;

        char GetPathSeparator() const;

    protected:
        std::string m_root;

    };//class DirectoryObject
}
