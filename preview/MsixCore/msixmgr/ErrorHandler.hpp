#pragma once

#include <windows.h>
#include <string>
#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
class ErrorHandler : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    /// Intentionally skipping implementation of ExecuteForRemoveRequest()
    /// since there is no use case as of now to handle an error in case of a remove request

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ErrorHandler() {}

private:
    MsixRequest * m_msixRequest = nullptr;

    ErrorHandler() {}
    ErrorHandler(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Prepares and sends msixrequest to uninstall the package in case cancel button was clicked during installation
    HRESULT RemovePackage(std::wstring packageFullName);
};
}