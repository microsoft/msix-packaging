#pragma once

#include<windows.h>
#include <afx.h>
#include <string>

namespace ErrorMessageHelper
{
    std::wstring GetErrorMessageFromHRESULT(
        HRESULT hresult)
    {
        CString strMessage;
        std::wstring errorMessage;
        try
        {
            // use FormatMessage to get a system-defined error message
            LPTSTR lpMsgBuf = NULL;
            DWORD dw = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                hresult,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0,
                NULL);
            if (dw > 0) {
                strMessage.Append(lpMsgBuf);
                LocalFree(lpMsgBuf);
            }
            errorMessage = strMessage.GetString();
        }
        catch (...)
        {
            errorMessage = L"Error Description could not be found";
        }

        return errorMessage;
    }

}