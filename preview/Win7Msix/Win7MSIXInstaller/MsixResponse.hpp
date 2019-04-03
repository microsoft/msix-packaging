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

    /// HResult to be populated by handlers in the msix response
    HRESULT m_hresultTextCode;

    /// Detailed text status of the msix response
    PCWSTR m_textStatus;

public:

    static HRESULT Make(MsixResponse** outInstance);

    /// Method used to return the status of the cancel button
    ///
    /// @return variable indicating the state of cancel button
    bool GetIsInstallCancelled()
    {
        return m_isInstallCancelled;
    }

    /// Method used to set the status of the cancel button if 'Cancel' is clicked
    void CancelRequest()
    {
        m_isInstallCancelled = true;
    }

    /// Set Hresult code as passed by handlers in the msix response(not being set currently)
    ///
    /// @param hresult - hresult value passed
    inline void SetHResultTextCode(HRESULT hresult)
    {
        m_hresultTextCode = hresult;
    }

    /// Get the Hresult value in an msix response
    ///
    /// @return Hresult code as set in the response
    inline HRESULT GetHResultTextCode()
    {
        return m_hresultTextCode;
    }

    /// Sets the detailed text status in the msix response object
    ///
    /// @param textStatus - the textStatus to be set 
    inline void SetTextStatus(PCWSTR textStatus)
    {
        m_textStatus = textStatus;
    }

    /// Returns the detailed text status as set in the msix response object
    ///
    /// @return textStatus in the msix response
    inline PCWSTR GetTextStatus()
    {
        return m_textStatus;
    }
};