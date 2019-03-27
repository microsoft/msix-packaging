#pragma once
#include "PackageInfo.hpp"
#include "FilePaths.hpp"
class UI;

enum OperationType
{
    Undefined = 0,
    Add = 1,
    Remove = 2,
    FindPackage = 3,
    FindAllPackages = 4,
};

enum Flags
{
    NoFlags = 0,
    QuietUX = 0x1,
};
DEFINE_ENUM_FLAG_OPERATORS(Flags);

/// MsixRequest represents what this instance of the executable will be doing and tracks the state of the current operation
class MsixRequest
{
private:
    /// Should always be available via constructor
    std::wstring m_packageFilePath;
    std::wstring m_packageFullName;
    MSIX_VALIDATION_OPTION m_validationOptions = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
    Flags m_flags = NoFlags;
    OperationType m_operationType = Add;
    FilePathMappings m_filePathMappings;

    /// Filled by PopulatePackageInfo
    AutoPtr<PackageInfo> m_packageInfo;

    /// Filled in by CreateAndShowUI 
    AutoPtr<UI> m_UI;

    ///Variable used to indicate if add package request was cancelled during installation
    bool m_isInstallCancelled = false;

public:
    static HRESULT Make(OperationType operationType, Flags flags, std::wstring packageFilePath, std::wstring packageFullName, MSIX_VALIDATION_OPTION validationOption, MsixRequest** outInstance);

    /// The main function processes the request based on whichever operation type was requested and then
    /// going through the sequence of individual handlers.
    HRESULT ProcessRequest();

    /// Called by PopulatePackageInfo
    void SetPackageInfo(PackageInfo* packageInfo);

    /// Called by CreateAndShowUI 
    void SetUI(UI* ui);

    // Getters
    MSIX_VALIDATION_OPTION GetValidationOptions() { return m_validationOptions; }
    PCWSTR GetPackageFilePath() { return m_packageFilePath.c_str(); }
    PCWSTR GetPackageFullName() { return m_packageFullName.c_str(); }
    
    FilePathMappings* GetFilePathMappings() { return &m_filePathMappings; }

    /// @return can return null if called before PopulatePackageInfo.
    PackageInfo* GetPackageInfo() { return m_packageInfo; }

    /// @return can return null if called before CreateAndShowUI or if Flags::QuietUX was passed in and there is no UI.
    UI* GetUI() { return m_UI; }
    bool IsQuietUX() { return (m_flags & Flags::QuietUX) == Flags::QuietUX; }

    /// Prepares and sends msixrequest to uninstall the package in case cancel button was clicked during installation
    HRESULT RemovePackage(std::wstring packageFullName);
    
    bool IsRemove()
    {
        return m_operationType == OperationType::Remove;
    }

    bool AllowSignatureOriginUnknown()
    {
        m_validationOptions = static_cast<MSIX_VALIDATION_OPTION>(m_validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN);
        return true;
    }

    bool GetIsInstallCancelled()
    {
        return m_isInstallCancelled;
    }

    //Setters
    void CancelRequest()
    {
        m_isInstallCancelled = true;
    }

private:
    /// FilePath Mappings maps the VFS tokens (e.g. Windows) to the actual folder on disk (e.g. C:\windows)
    HRESULT InitializeFilePathMappings();

    /// This handles FindAllPackages operation and finds all packages installed by the Win7MSIXInstaller
    HRESULT FindAllPackages();

    /// This handles Add operation and proceeds through each of the AddSequenceHandlers to install the package
    HRESULT ProcessAddRequest();

    /// This handles Remove operation and proceeds through each of the RemoveSequenceHandlers to uninstall the package
    HRESULT ProcessRemoveRequest();

    /// This handles FindPackage operation and displays the package info for a given package.
    /// @return E_NOT_SET when the package could not be found
    HRESULT DisplayPackageInfo();
};

