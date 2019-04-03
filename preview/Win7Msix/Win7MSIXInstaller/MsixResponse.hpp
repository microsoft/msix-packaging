#pragma once
#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"

/// The response class tracks the response state of the msix deploy operation
/// (if the operation was cancelled, progress bar updates)
class MsixResponse
{
private:

    /// Variable used to indicate if add package request was cancelled during installation
    bool m_isInstallCancelled = false;

    /// Error Code of msix response
    HRESULT errorCode;

    /// Error Text of msix response
    PCWSTR errorText;

public:

    static HRESULT Make(MsixResponse** outInstance);

    /// @return variable indicating the state of cancel button
    bool GetIsInstallCancelled() { return m_isInstallCancelled; }

    /// Set if Cancel button is clicked
    void CancelRequest() { m_isInstallCancelled = true; }

    /// Set error code of response
    void SetErrorCode(HRESULT code) { errorCode = code; }

    /// @return ErrorCode of response
    HRESULT GetErrorCode() { return errorCode; }

    /// Set response error text
    void SetErrorText(PCWSTR errorText) { errorText = errorText; }

    /// @return ErrorText of response
    PCWSTR GetErrorText() { return errorText; }
};