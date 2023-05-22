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
        WORD facility = HRESULT_FACILITY(hresult);
        CComPtr<IErrorInfo> iei;
        if (S_OK == GetErrorInfo(0, &iei) && iei) {
            // get the error description from the IErrorInfo
            BSTR bstr = NULL;
            if (SUCCEEDED(iei->GetDescription(&bstr))) {
                // append the description to our label
                strMessage.Append(bstr);
                // done with BSTR, do manual cleanup
                SysFreeString(bstr);
            }
        }
        else if (facility == FACILITY_ITF) {
            // interface specific - no standard mapping available
            strMessage.Append(_T("FACILITY_ITF - This error is interface specific."));
        }
        else {
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
        }

        std::wstring errorMessage = strMessage.GetString();
        return errorMessage;
    }

}