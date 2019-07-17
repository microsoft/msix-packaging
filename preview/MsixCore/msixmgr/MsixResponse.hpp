#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include "IMsixResponse.hpp"

#include "GeneralUtil.hpp"
namespace MsixCoreLib
{
/// The response class tracks the response state of the msix deploy operation
/// (if the operation was cancelled, progress bar updates)
class MsixResponse: public IMsixResponse
{
private:

    /// Variable used to indicate if add package request was cancelled during installation
    bool m_isInstallCancelled = false;

    /// HResult to be populated by handlers in the msix response
    HRESULT m_hresultTextCode = 0;

    /// Detailed text status of the msix response
    std::wstring m_textStatus;

    /// Progress percentage of the msix deployment
    float m_percentage;

    InstallationStep m_status = InstallationStepUnknown;

    /// Callback to indicate how the progress of the installation
    std::function<void(const IMsixResponse&)> m_callback;

public:

    /// Method used to return the status of the cancel button
    ///
    /// @return variable indicating the state of cancel button
    bool GetIsInstallCancelled()
    {
        return m_isInstallCancelled;
    }

    /// Method used to set the status of the cancel button if 'Cancel' is clicked
    virtual void CancelRequest()
    {
        m_isInstallCancelled = true;
    }


    /// Get the Hresult value in an msix response
    ///
    /// @return Hresult code as set in the response
    virtual inline HRESULT GetHResultTextCode() const
    {
        return m_hresultTextCode;
    }

    /// Sets the detailed text status in the msix response object
    ///
    /// @param textStatus - the textStatus to be set 
    inline void SetTextStatus(std::wstring textStatus)
    {
        m_textStatus = textStatus;
    }

    /// Returns the detailed text status as set in the msix response object
    ///
    /// @return textStatus in the msix response
    virtual inline std::wstring GetTextStatus() const
    {
        return m_textStatus;
    }

    /// Update response and send a callback to the UI
    ///
    /// @param status - current step of the installation
    /// @param percentage - installation progresss percentage
    void Update(InstallationStep status, float percentage);

    /// Update response to indicate an error status
    ///
    /// @param errorCode - code associated to the error
    /// @param errorText - description of the error
    void SetErrorStatus(HRESULT errorCode, std::wstring errorText);

    /// Set a callback for the UI
    ///
    /// @param callback - callback called when status or percentage change
    virtual void SetCallback(std::function<void(const IMsixResponse&)> callback);

    /// Get the installation progress percentage 
    ///
    /// @return the percentage (from 0 to 100)
    virtual inline float GetPercentage() const
    {
        return m_percentage;
    }

    /// Get the installation status
    ///
    /// @return the installation status
    virtual inline InstallationStep GetStatus() const
    {
        return m_status;
    }
};
}