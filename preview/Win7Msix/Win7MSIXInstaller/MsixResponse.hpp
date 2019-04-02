#pragma once
#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"

class MsixResponse
{
private:

    ///Variable used to indicate if add package request was cancelled during installation
    bool m_isInstallCancelled = false;

    HRESULT errorCode;

    PCWSTR errorText;

public:

    static HRESULT Make(MsixResponse** outInstance);

    //Setters and Getters
    bool GetIsInstallCancelled()
    {
        return m_isInstallCancelled;
    }

    void CancelRequest()
    {
        m_isInstallCancelled = true;
    }

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






