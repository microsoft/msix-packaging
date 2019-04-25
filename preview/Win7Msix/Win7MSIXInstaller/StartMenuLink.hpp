#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Handles adding/removing the shortcut in the start menu to launch the application
class StartMenuLink : IPackageHandler
{
public:
    /// Creates the start menu shortcut to the executable for the application
    HRESULT ExecuteForAddRequest();

    /// Removes the start menu link
    HRESULT ExecuteForRemoveRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~StartMenuLink() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    StartMenuLink() {}
    StartMenuLink(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Creates the Shell link for the start menu shortcut.
    /// Uses the Shell's IShellLink and IPersistFile interfaces
    /// to create and store a shortcut to the specified object.
    /// @param targetFilePath - full file path to the application exe
    /// @param linkFilePath - full file path to the shortcut .lnk file
    /// @param description - description of the Shell link, stored in the Comment field of the link properties.
    /// @param appUserModelId- Application User Model ID, needed to display toasts.
    HRESULT CreateLink(PCWSTR targetFilePath, PCWSTR linkFilePath, PCWSTR description, PCWSTR appUserModelId = NULL);
};
}
