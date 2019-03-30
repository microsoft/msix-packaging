#pragma once
#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"

class MsixResponse
{
private:
    HRESULT errorCode;

    PCWSTR errorText;

public:

    static HRESULT Make(MsixResponse** outInstance);

    //Setters and Getters
    void SetErrorCode(HRESULT code)
    {
        errorCode = code;
    }

    HRESULT GetErrorCode()
    {
        return errorCode;
    }

    void setErrorText(PCWSTR errorText)
    {
        errorText = errorText;
    }

    PCWSTR getErrorText()
    {
        return errorText;
    }

};






