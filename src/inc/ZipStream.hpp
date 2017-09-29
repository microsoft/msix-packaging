#pragma once

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ZipFileStream.hpp"

#include <vector>
#include <map>
#include <memory>

namespace xPlat {

    class ZipException : public ExceptionBase
    {
    public:
        enum Error : std::uint32_t
        {
            InvalidHeader = 1,
            FieldOutOfRange = 2,
            InvalidEndOfCentralDirectoryRecord = 3,
            InvalidZip64CentralDirectoryLocator = 4,
            InvalidZip64CentralDirectoryRecord = 5,
        };

        ZipException(std::string message, Error error) : reason(message), ExceptionBase(ExceptionBase::Facility::ZIP)
        {
            SetLastError(error);
        }
        std::string reason;
    };

    // This represents a raw stream over a.zip file.
    class ZipStream
    {
    public:
        ZipStream(StreamPtr&& stream) : stream(std::move(stream)) { }

        void Read();

        std::vector<std::string> GetFileNames();


    protected:
        StreamPtr stream;
        std::map<std::string, std::shared_ptr<ZipFileStream>> containedFiles;

    };//class ZipStream
}