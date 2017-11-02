#pragma once

#include <cstdint>
#include <string>
#include <exception>
#include <cassert>
#include <functional>

#include "AppxWindows.hpp"

namespace xPlat {

    static const std::uint32_t ERROR_FACILITY = 0x8BAD0000;   // Facility 2989

    // defines error codes
    enum class Error : std::uint32_t
    {
        //
        // Win32 error codes
        //
        OK                          = 0,
        NotSupported                = 0x80070032,
        InvalidParameter            = 0x80070057,
        NotImplemented              = 0x80070078,
        OutOfMemory                 = 0x80000002,
        Unexpected                  = 0x8000ffff,
        NoInterface                 = 0x80000004,

        //
        // xPlat specific error codes
        //

        // Basic file errors
        FileOpen                    = ERROR_FACILITY + 0x0001,
        FileSeek                    = ERROR_FACILITY + 0x0002,
        FileRead                    = ERROR_FACILITY + 0x0003,
        FileWrite                   = ERROR_FACILITY + 0x0003,
        FileCreateDirectory         = ERROR_FACILITY + 0x0004,
        FileSeekOutOfRange          = ERROR_FACILITY + 0x0005,

        // Zip format errors
        ZipCentralDirectoryHeader   = ERROR_FACILITY + 0x0011,
        ZipLocalFileHeader          = ERROR_FACILITY + 0x0012,
        Zip64EOCDRecord             = ERROR_FACILITY + 0x0013,
        Zip64EOCDLocator            = ERROR_FACILITY + 0x0014,
        ZipEOCDRecord               = ERROR_FACILITY + 0x0015,
        ZipHiddenData               = ERROR_FACILITY + 0x0016,

        // Inflate errors
        InflateInitialize           = ERROR_FACILITY + 0x0021,
        InflateRead                 = ERROR_FACILITY + 0x0022,
        InflateCorruptData          = ERROR_FACILITY + 0x0023,

        // Signature errors
        AppxSignatureInvalid        = ERROR_FACILITY + 0x0030,
        // AppxPackage format errors
        AppxMissingSignatureP7X     = ERROR_FACILITY + 0x0031,
        AppxMissingContentTypesXML  = ERROR_FACILITY + 0x0032,
        AppxMissingBlockMapXML      = ERROR_FACILITY + 0x0033,
        AppxMissingAppxManifestXML  = ERROR_FACILITY + 0x0034,
        AppxDuplicateFootprintFile  = ERROR_FACILITY + 0x0035,
    };

    // Defines a common exception type to throw in exceptional cases.  DO NOT USE FOR FLOW CONTROL!
    // Throwing xPlat::Exception will break into the debugger on chk builds to aid debugging
    class Exception : public std::exception
    {
    public:
        Exception(Error error) : m_code(static_cast<std::uint32_t>(error))
        {}

        Exception(std::uint32_t error) : m_code(0x80070000 + error)
        {}

        Exception(Error error, std::string& message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {}

        Exception(Error error, const char* message) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {}

        Exception(HRESULT error, std::string& message) :
            m_code(error),
            m_message(message)
        {}

        Exception(HRESULT error, const char* message) :
            m_code(error),
            m_message(message)
        {}            

        uint32_t        Code() { return m_code; }
        std::string&    Message() { return m_message; }

    protected:
        std::uint32_t   m_code;
        std::string     m_message;
    };

    class Win32Exception : public Exception
    {
    public:
        Win32Exception(DWORD error, std::string& message) :
            Exception(0x80070000 + error, message)
        {}

        Win32Exception(DWORD error, const char* message) :
            Exception(0x80070000 + error, message)
        {}
    };

    // Provides an ABI exception boundary with parameter validation
    template <class Lambda>
    inline unsigned int ResultOf(Lambda lambda)
    {
        unsigned int result = 0;
        try
        {
            lambda();
        }
        catch (xPlat::Exception& e)
        {
            result = e.Code();
        }
        catch (std::bad_alloc&)
        {
            result = static_cast<unsigned int>(xPlat::Error::OutOfMemory);
        }
        catch (std::exception&)
        {
            result = static_cast<unsigned int>(xPlat::Error::Unexpected);
        }

        return result;
    }
}

// Helper to make code more terse and more readable at the same time.
#define ThrowErrorIfNot(c, a, m)     \
{                                    \
    if (!(a))                        \
    {                                \
        assert(false);               \
        throw xPlat::Exception(c,m); \
    }                                \
}

#define ThrowWin32ErrorIfNot(c, a, m)       \
{                                           \
    if (!(a))                               \
    {                                       \
        assert(false);                      \
        throw xPlat::Win32Exception(c,m);   \
    }                                       \
}

#define ThrowErrorIf(c, a, m) ThrowErrorIfNot(c,!(a), m)

#define ThrowHrIfFailed(a)                              \
{                                                       \
    HRESULT hr = a;                                     \
    if (FAILED(hr))                                     \
    {   assert(false);                                  \
        throw xPlat::Exception(hr, "COM Call failed");  \
    }                                                   \
}
