#pragma once

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"

#include <vector>
#include <map>
#include <memory>

namespace xPlat {

    class ZipException : public ExceptionBase
    {
    public:
        enum class Error : std::uint32_t
        {
            InvalidHeader                       = 1,
            FieldOutOfRange                     = 2,
            InvalidEndOfCentralDirectoryRecord  = 3,
            InvalidZip64CentralDirectoryLocator = 4,
            InvalidZip64CentralDirectoryRecord  = 5,
            InvalidCentralDirectoryHeader       = 6,
            HiddenDataBetweenLastCDHandEoCD     = 7,
            InvalidLocalFileHeader              = 8,
        };

        ZipException(std::string message, Error error) : reason(message), ExceptionBase(ExceptionBase::SubFacility::ZIP)
        {
            SetLastError(static_cast<std::uint32_t>(error));
        }
        std::string reason;
    };

    // forward declarations
    class CentralDirectoryFileHeader;
    class LocalFileHeader;

    // This represents a raw stream over a.zip file.
    class ZipObject : public StorageObject
    {
    public:
        ZipObject(StreamBase* stream);

        // StorageObject methods
        virtual std::vector<std::string>    GetFileNames() override;
        virtual std::shared_ptr<StreamBase> GetFile(std::string& fileName) override;
        virtual void                        RemoveFile(std::string& fileName) override;
        virtual std::shared_ptr<StreamBase> OpenFile(std::string& fileName, FileStream::Mode mode) override;
        virtual void                        CommitChanges() override;

    protected:
        std::map<std::string, std::shared_ptr<StreamBase>>                   m_streams;
        std::map<std::string, std::shared_ptr<CentralDirectoryFileHeader>>   m_centralDirectory;

        // TODO: change to uint64_t when adding full zip64 support
        std::map<std::uint32_t, std::shared_ptr<LocalFileHeader>>            m_fileRepository;

    };//class ZipObject
}
