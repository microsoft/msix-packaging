#pragma once

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"

#include <memory>
#include <string>
#include <limits>
#include <functional>

namespace xPlat {

    class ZipException : public ExceptionBase
    {
    public:
        enum Error : std::uint32_t
        {
            InvalidHeader = 1,
            FieldOutOfRange = 2,
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

    protected:
        StreamPtr stream;
    };//class ZipStream
}