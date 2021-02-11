#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Determines if the incoming add request is actually an update to an existing package.
/// If it is, it'll remove the outdated package
class ProcessPotentialUpdate : IPackageHandler
{
public:
    /// Determines if the incoming add request is actually an update to an existing package.
    /// If it is, it'll remove the outdated package. This handler is only run on an AddRequest.
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ProcessPotentialUpdate() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    ProcessPotentialUpdate() {}
    ProcessPotentialUpdate(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Synchronously removes the outdated package before allowing the current request to proceed
    HRESULT RemovePackage(std::wstring packageFullName);
};
}