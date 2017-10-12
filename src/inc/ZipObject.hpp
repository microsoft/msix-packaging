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

        ZipException(std::string&& message, Error error) : reason(std::move(message)), ExceptionBase(ExceptionBase::SubFacility::ZIP)
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
        ZipObject(std::unique_ptr<StreamBase>&& stream);

        // StorageObject methods
        std::string                 GetPathSeparator() override;
        std::vector<std::string>    GetFileNames() override;
        std::shared_ptr<StreamBase> GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        std::shared_ptr<StreamBase> OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                        CommitChanges() override;

    protected:
        std::unique_ptr<StreamBase>                                          m_stream;
        std::map<std::string, std::shared_ptr<StreamBase>>                   m_streams;
        std::map<std::string, std::shared_ptr<CentralDirectoryFileHeader>>   m_centralDirectory;

        // TODO: change to uint64_t when adding full zip64 support
        std::map<std::uint32_t, std::shared_ptr<LocalFileHeader>>            m_fileRepository;

    };//class ZipObject
}
