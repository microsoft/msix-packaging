//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <cstdint>
#include <string>
#include <exception>
#include <cassert>
#include <functional>
#include <sstream>

#include "Log.hpp"
#include "MSIXWindows.hpp"
#include "MsixErrors.hpp"

namespace MSIX {

    // Defines a common exception type to throw in exceptional cases.  DO NOT USE FOR FLOW CONTROL!
    // Throwing MSIX::Exception will break into the debugger on chk builds to aid debugging
    class Exception : public std::exception
    {
    public:
        Exception(std::string& message, Error error) :
            m_code(static_cast<std::uint32_t>(error)),
            m_message(message)
        {
            Global::Log::Append(Message());
        }

        Exception(std::string& message, HRESULT error) :
            m_code(error),
            m_message(message)
        {
            Global::Log::Append(Message());
        }

        uint32_t            Code() { return m_code; }
        const std::string&  Message() { return m_message; }

    protected:
        std::uint32_t   m_code;
        std::string     m_message;
    };

    class Win32Exception final : public Exception
    {
    public:
        Win32Exception(std::string& message, DWORD error) : Exception(message, 0x80070000 + error)
        {
            Global::Log::Append(Message());
        }
    };

    // Provides an ABI exception boundary with parameter validation
    #define CATCH_RETURN()                                                  \
    catch (MSIX::Exception& e)                                              \
    {   return static_cast<HRESULT>(e.Code());                              \
    }                                                                       \
    catch (std::bad_alloc&)                                                 \
    {   return static_cast<HRESULT>(MSIX::Error::OutOfMemory);              \
    }                                                                       \
    catch (...)                                                             \
    {   return static_cast<HRESULT>(MSIX::Error::Unexpected);               \
    }

    template <typename E, class C>
    #ifdef WIN32
    __declspec(noinline)
    __declspec(noreturn)
    #endif
    void 
    #ifndef WIN32
    __attribute__(( noinline, cold, noreturn )) 
    #endif
    RaiseException(const int line, const char* const file, const char* details, C c)
    {
        assert(false);
        std::ostringstream builder;
        if (details) { builder << details << "\n"; }
        builder << "Call failed in " << file << " on line " << line;
        std::string message = builder.str();
        throw E(message, c);
    }
    
    #ifdef WIN32
    __declspec(noinline)
    #endif
    void 
    #ifndef WIN32
    __attribute__(( noinline)) 
    #endif
    RaiseExceptionIfFailed(HRESULT hr, const int line, const char* const file);
}

// Helper to make code more terse and more readable at the same time.
#define ThrowError(c)  { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, nullptr, c); }
#define UNEXPECTED     { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, nullptr, Error::Unexpected); }
#define NOTSUPPORTED   { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, nullptr, Error::NotSupported); }
#define NOTIMPLEMENTED { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, nullptr, Error::NotImplemented); }

#define ThrowErrorIfNot(c, a, m)      if (!(a)) { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, m, c); }
#define ThrowWin32ErrorIfNot(c, a, m) if (!(a)) { MSIX::RaiseException<MSIX::Win32Exception>(__LINE__, __FILE__, m, c); }
#define ThrowErrorIf(c, a, m) ThrowErrorIfNot(c,!(a), m)
#define ThrowErrorAndLog(c, m) { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, m, c); }

#define ThrowHrIfFailed(a) MSIX::RaiseExceptionIfFailed(a, __LINE__, __FILE__);

#ifdef WIN32
    #define ThrowHrIfFalse(a, m)                                                                               \
    {   BOOL _result = a;                                                                                      \
        if (!_result)                                                                                          \
        {   MSIX::RaiseException<MSIX::Exception> (__LINE__, __FILE__, m, HRESULT_FROM_WIN32(GetLastError())); \
        }                                                                                                      \
    }
    #define ThrowLastErrorIf(a, m) { if (a) { MSIX::RaiseException<MSIX::Exception> (__LINE__, __FILE__, m, HRESULT_FROM_WIN32(GetLastError())); }}
#endif
