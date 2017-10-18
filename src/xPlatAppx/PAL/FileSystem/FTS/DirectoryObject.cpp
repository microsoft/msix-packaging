#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "DirectoryObject.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fts.h>

namespace xPlat {
    
    std::vector<std::string> DirectoryObject::GetFileNames()
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw Exception(Error::NotImplemented);
    }
    
    std::shared_ptr<StreamBase> DirectoryObject::GetFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw Exception(Error::NotImplemented);
    }
    
    void DirectoryObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw Exception(Error::NotImplemented);
    }
    
    std::string DirectoryObject::GetPathSeparator() { return "/"; }
    
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
            p++;
        }
    }
    
    std::shared_ptr<StreamBase> DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        std::string name = m_root + "/" + fileName;
        auto lastSlash = name.find_last_of("/");
        std::string path = name.substr(0, lastSlash);
        mkdirp(path);
        auto result = m_streams[fileName] = std::make_unique<FileStream>(std::move(name), mode);
        return result;
    }
    
    void DirectoryObject::CommitChanges()
    {
        m_streams.clear();
    }
}
