#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "DirectoryObject.hpp"
#include <fts.h>

namespace xPlat {
    
    class FTSException : public ExceptionBase
    {
    public:
        FTSException(int rc) : ExceptionBase(0x8f75, SubFacility::NONE)
        {
            SetLastError(static_cast<std::uint32_t>(rc));
        }
    };
    
    std::vector<std::string> DirectoryObject::GetFileNames()
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw NotImplementedException();
    }
    
    std::shared_ptr<StreamBase> DirectoryObject::GetFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw NotImplementedException();
    }
    
    void DirectoryObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes
        throw NotImplementedException();
    }
    
    std::shared_ptr<StreamBase> DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        std::string name = fileName; // TODO: do we have to create directories as we go like how we do on Windows?
        auto result = m_streams[fileName] = std::make_unique<FileStream>(std::move(name), mode);
        return result;
    }
    
    void DirectoryObject::CommitChanges()
    {
        m_streams.clear();
    }
}
