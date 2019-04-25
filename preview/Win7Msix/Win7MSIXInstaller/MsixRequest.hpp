#pragma once
#include "Package.hpp"
#include "FilePaths.hpp"
#include "MsixResponse.hpp"

namespace MsixCoreLib
{
enum OperationType
{
    Add = 1,
    Remove = 2,
};

/// MsixRequest represents what this instance of the executable will be doing and tracks the state of the current operation
class MsixRequest
{
private:
    /// Should always be available via constructor
    std::wstring m_packageFilePath;
    std::wstring m_packageFullName;
    MSIX_VALIDATION_OPTION m_validationOptions = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
    OperationType m_operationType = Add;

    /// Filled by PopulatePackageInfo
    std::shared_ptr<PackageBase> m_packageInfo;

    /// MsixResponse object populated by handlers
    std::shared_ptr<MsixResponse> m_msixResponse;

protected:
    MsixRequest() {}
public:
    static HRESULT Make(OperationType operationType, const std::wstring & packageFilePath, std::wstring packageFullName, MSIX_VALIDATION_OPTION validationOption, MsixRequest** outInstance);

    /// The main function processes the request based on whichever operation type was requested and then
    /// going through the sequence of individual handlers.
    HRESULT ProcessRequest();

    bool IsRemove()
    {
        return m_operationType == OperationType::Remove;
    }

    bool AllowSignatureOriginUnknown()
    {
        m_validationOptions = static_cast<MSIX_VALIDATION_OPTION>(m_validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN);
        return true;
    }

    inline MSIX_VALIDATION_OPTION GetValidationOptions() { return m_validationOptions; }
    inline PCWSTR GetPackageFilePath() { return m_packageFilePath.c_str(); }
    inline PCWSTR GetPackageFullName() { return m_packageFullName.c_str(); }

    /// Retrieves the msixResponse object
    ///
    /// @return m_msixResponse object
    std::shared_ptr<MsixResponse> GetMsixResponse() { return m_msixResponse; }

    /// Called by PopulatePackageInfo
    void SetPackageInfo(std::shared_ptr<PackageBase> packageInfo);
    std::wstring GetPackageDirectoryPath();

    /// @return can return null if called before PopulatePackageInfo.
    std::shared_ptr<PackageBase> GetPackageInfo() { return m_packageInfo; }

private:
    /// This handles Add operation and proceeds through each of the AddSequenceHandlers to install the package
    HRESULT ProcessAddRequest();

    /// This handles Remove operation and proceeds through each of the RemoveSequenceHandlers to uninstall the package
    HRESULT ProcessRemoveRequest();

};
}