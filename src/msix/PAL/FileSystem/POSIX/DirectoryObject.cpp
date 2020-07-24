//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "DirectoryObject.hpp"
#include "MsixFeatureSelector.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fts.h>
#include <dirent.h>
#include <map>

namespace MSIX {

    template<class Lambda>
    void WalkDirectory(const std::string& root, Lambda& visitor)
    {
        static std::string dot(".");
        static std::string dotdot("..");

        std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(root.c_str()), closedir);
        ThrowErrorIf(Error::FileNotFound, dir.get() == nullptr, "Invalid directory");
        struct dirent* dp;
        // TODO: handle junction loops
        while((dp = readdir(dir.get())) != nullptr)
        {
            std::string fileName = std::string(dp->d_name);
            std::string child = root + "/" + fileName;
            if (dp->d_type == DT_DIR)
            {
                if ((fileName != dot) && (fileName != dotdot))
                {
                    WalkDirectory(child, visitor);
                }
            }
            else
            {
                // TODO: ignore .DS_STORE for mac?
                struct stat sb;
                ThrowErrorIf(Error::Unexpected, stat(child.c_str(), &sb) == -1, std::string("stat call failed" + std::to_string(errno)).c_str());
                if (!visitor(root, std::move(fileName), static_cast<std::uint64_t>(sb.st_mtime)))
                {
                    break;
                }
            }
        }
    }

    std::vector<std::string> DirectoryObject::GetFileNames(FileNameOptions)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        NOTIMPLEMENTED;
    }

    #define DEFAULT_MODE S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
    void mkdirp(std::string& path, size_t startPos = 0, mode_t mode = DEFAULT_MODE)
    {
        char* p = &path[startPos];
        if (*p == '/') { p++; }
        while (*p != '\0')
        {
            while (*p != '\0' && *p != '/') { p++; }

            char v = *p;
            *p = '\0';
            ThrowErrorIfNot(Error::FileCreateDirectory,(mkdir(path.c_str(), mode) != -1 || errno == EEXIST), path.c_str());
            *p = v;
            if (*p != '\0') {p++;}
        }
    }

    const char* DirectoryObject::GetPathSeparator() { return "/"; }

    DirectoryObject::DirectoryObject(const std::string& root, bool createRootIfNecessary) : m_root(root)
    {
        auto lastOf = m_root.find_last_of(GetPathSeparator());
        if (lastOf == m_root.length() - 1)
        {
            m_root = m_root.substr(0, m_root.length() - 1);
        }

        if (createRootIfNecessary)
        {
            mkdirp(m_root);
        }
    }

    ComPtr<IStream> DirectoryObject::OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode)
    {
        std::string name = m_root + GetPathSeparator() + fileName;
        auto lastSlash = name.find_last_of(GetPathSeparator());
        std::string path = name.substr(0, lastSlash);
        mkdirp(path, m_root.size());
        auto result = ComPtr<IStream>::Make<FileStream>(std::move(name), mode);
        return result;
    }

    std::multimap<std::uint64_t, std::string> DirectoryObject::GetFilesByLastModDate()
    {
        THROW_IF_PACK_NOT_ENABLED
        std::multimap<std::uint64_t, std::string> files;
        auto rootSize = m_root.size() + 1; // plus separator
        auto lamdba = [&](
                std::string root,
                std::string&& name,
                std::uint64_t size)
           {
                std::string fileName = root + GetPathSeparator() + name;
                // root contains the top level directory, which we don't need
                fileName = fileName.substr(rootSize);
                files.insert(std::make_pair(size, std::move(fileName)));
                return true;
           };
        WalkDirectory(m_root, lamdba);
        return files;
    }
}
