#pragma once

#include <cstdint>
#include <string>
#include <exception>
#include <cassert>

namespace xPlat {

    class ExceptionBase : public std::exception
    {
    public:
        enum SubFacility : uint32_t {
            NONE    = 0x0000,
            FILE    = 0x1000,
            ZIP     = 0x2000,
            INFLATE = 0x4000,
            MAX     = 0x0FFF      // always last, always defines bytes reserved for forwarding errors
        };

        ExceptionBase()
        {
            assert(false); // for debugging purposes.
        }

        ExceptionBase(SubFacility subFacility) : subFacility(subFacility) {}
        ExceptionBase(uint32_t headerOveride, SubFacility subFacility) : header(headerOveride), subFacility(subFacility) {}

        void SetLastError(uint32_t error)
        {
            code = header + subFacility + (error & SubFacility::MAX);
        }

        uint32_t Code() { return code; }

    protected:
        SubFacility subFacility = SubFacility::NONE;
        uint32_t    header = 0x8BAD0000;   // SubFacility 2989
        uint32_t    code   = 0xFFFFFFFF;   // by default, something very bad happened...
    };

    class NotImplementedException   : public ExceptionBase { public: NotImplementedException()  { SetLastError(1); } };
    class NotSupportedException     : public ExceptionBase { public: NotSupportedException()    { SetLastError(2); } };
    class InvalidArgumentException  : public ExceptionBase { public: InvalidArgumentException() { SetLastError(3); } };
    class IOException               : public ExceptionBase { public: IOException()              { SetLastError(4); } };
}