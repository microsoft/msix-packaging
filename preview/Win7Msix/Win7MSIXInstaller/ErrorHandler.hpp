#pragma once

#include <windows.h>
#include <string>
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class ErrorHandler : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

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