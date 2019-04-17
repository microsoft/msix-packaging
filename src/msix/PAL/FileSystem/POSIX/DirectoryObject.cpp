//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "DirectoryObject.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fts.h>
#include <map>

namespace MSIX {

    void WalkDirectory(const std::string& root, Lambda& visitor)
    {
        static std::string dot(".");
        static std::string dotdot("..");

        std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(root.c_str()), closedir);
        ThrowErrorIf(Error::FileNotFound, dir.get() == nullptr, "Invalid directory");
        struct dirent* dp;
        while((dp = readdir(dir)) != nullptr)
        {
            std::string fileName = std::string(dp->d_name);
            if (dp->d_type == DT_DIR)
            {
                std::string child = root + "/" + fileName;
                if ((fileName != dot) && (fileName != dotdot))
                {
                    WalkDirectory(child, visitor);
                }
            }
            else
            {
                // TODO: ignore .DS_STORE for mac?
                struct stat sb;
                ThrowErrorIf(Error::Unexpected, stat(fullPath.c_str(), &sb) == -1, "stat call failed" + std::to_string(errno));
                if (!visitor(root, std::move(child), static_cast<std::uint64_t>(sb.st_mtime)))
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
    
    ComPtr<IStream> DirectoryObject::GetFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        NOTIMPLEMENTED;
    }

    #define DEFAULT_MODE S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
    void mkdirp(std::string& path, mode_t mode = DEFAULT_MODE)
    {
        char* p = const_cast<char*>(path.c_str());
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

    ComPtr<IStream> DirectoryObject::OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode)
    {
        std::string name = m_root + GetPathSeparator() + fileName;
        auto lastSlash = name.find_last_of(GetPathSeparator());
        std::string path = name.substr(0, lastSlash);
        mkdirp(path);
        auto result = ComPtr<IStream>::Make<FileStream>(std::move(name), mode);
        return result;
    }

    std::multimap<std::uint64_t, std::string> DirectoryObject::GetFilesByLastModDate()
    {
    #ifdef MSIX_PACK
        std::multimap<std::uint64_t, std::string> files;
        WalkDirectory(m_root, [&](
                std::string root,
                std::string&& name,
                std::uint64_t size)
            {
                if (name != "AppxManifest.xml") // should only add payload files to the map
                {
                    std::string fileName = root + GetPathSeparator() + name;
                    // fileName includes the root directory, which we dont want.
                    fileName = fileName.substr(fileName.find_first_of(GetPathSeparator()) + 1);
                    files.insert(std::make_pair(size, std::move(fileName)));
                }
                return true;
            });
        return files;
    #else
        NOTIMPLEMENTED;
    #endif // MSIX_PACK
    }
}
