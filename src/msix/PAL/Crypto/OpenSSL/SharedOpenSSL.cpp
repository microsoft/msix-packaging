//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "SharedOpenSSL.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace MSIX
{
    inline std::string GetOpenSSLErrString(const char* message)
    {
        ERR_load_crypto_strings();

        std::ostringstream strstr;

        if (message)
        {
            strstr << message << std::endl;
        }

        strstr << "OpenSSL Error Data:" << std::endl;

        unsigned long err = 0;
        do
        {
            const char* file{};
            int line{};
            const char* data{};
            int flags{};

            err = ERR_get_error_line_data(&file, &line, &data, &flags);

            if (err)
            {
                strstr << "  at " << file << '[' << line << ']';
                if (flags & ERR_TXT_STRING)
                {
                    strstr << " : " << data;
                }
                strstr << std::endl;

                strstr << "    " << ERR_error_string(err, nullptr) << std::endl;
            }
        } while (err);

        return strstr.str();
    }
    
    MSIX_NOINLINE(void) RaiseOpenSSLException(const char* message, const int line, const char* const file, DWORD error)
    {
        const char* messageToPass = nullptr;
        std::string messageStr;

        if (error == static_cast<DWORD>(Error::OutOfMemory))
        {
            messageToPass = message;
        }
        else
        {
            messageStr = GetOpenSSLErrString(message);
            messageToPass = messageStr.c_str();
        }

        MSIX::RaiseException<MSIX::Exception>(line, file, messageToPass, error);
    }
} // namespace MSIX
