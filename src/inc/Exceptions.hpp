#pragma once

#include <cstdint>
#include <string>
#include <exception>
#include <cassert>

namespace xPlat {

    static const std::uint32_t ERROR_FACILITY = 0x8BAD0000;   // Facility 2989

    // defines error codes
    enum class Error : std::uint32_t
    {
        // win32 error codes
        NotSupported                            = 0x80070032,
        InvalidParameter                        = 0x80070057,
        NotImplemented                          = 0x80070078,

        //
        // xplat specific error codes
        //

        // Basic file errors
        FileOpen                                = ERROR_FACILITY + 0x0001,
        FileSeek                                = ERROR_FACILITY + 0x0002,
        FileRead                                = ERROR_FACILITY + 0x0003,
        FileWrite                               = ERROR_FACILITY + 0x0003,

        // Zip errors
        ZipInvalidHeader                        = ERROR_FACILITY + 0x0011,
        ZipFieldOutOfRange                      = ERROR_FACILITY + 0x0012,
        ZipInvalidEndOfCentralDirectoryRecord   = ERROR_FACILITY + 0x0013,
        ZipInvalidZip64CentralDirectoryLocator  = ERROR_FACILITY + 0x0014,
        ZipInvalidZip64CentralDirectoryRecord   = ERROR_FACILITY + 0x0015,
        ZipInvalidCentralDirectoryHeader        = ERROR_FACILITY + 0x0016,
        ZipHiddenDataBetweenLastCDHandEoCD      = ERROR_FACILITY + 0x0017,
        ZipInvalidLocalFileHeader               = ERROR_FACILITY + 0x0018,
    };

    // Defines a common exception type to throw in exceptional cases.  DO NOT USE FOR FLOW CONTROL!
    // Throwing xPlat::Exception will break into the debugger on chk builds to aid debugging
    class Exception : public std::exception
    {
    public:
        Exception(Error error) : m_code(static_cast<std::uint32_t>(error))
        {
            assert(false);
        }

        Exception(Error error, std::string& message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(std::move(message))
        {
            assert(false);
        }

        Exception(Error error, const char* message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {
            assert(false);
        }

        Exception(std::uint32_t error) : m_code(ERROR_FACILITY + error)
        {
            assert(false);
        }

        Exception(std::uint32_t error, std::string& message) :
            m_code(ERROR_FACILITY + error),
            m_message(std::move(message))
        {
            assert(false);
        }

        uint32_t        Code()      { return m_code; }
        std::string&    Message()   { return m_message; }

    protected:
        std::uint32_t   m_code;
        std::string     m_message;
    };

    // Helpers to make code more terse and more readable at the same time.  
    #define Assert(c, a)   {if (!(a)) throw Exception(c);}
    #define Assert(c, a, m) {if (!(a)) throw Exception(c,m);}
}