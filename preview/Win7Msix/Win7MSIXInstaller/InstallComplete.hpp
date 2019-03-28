#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class InstallComplete : IPackageHandler
{
public:
    /// Send install complete message to UI if cancel button was not clicked, else uninstalls the app
	HRESULT ExecuteForAddRequest();

	static const PCWSTR HandlerName;
	static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
	~InstallComplete() {}
private:
	MsixRequest * m_msixRequest = nullptr;

    /// Prepares and sends msixrequest to uninstall the package in case cancel button was clicked during installation
    HRESULT RemovePackage(std::wstring packageFullName);

	InstallComplete() {}
	InstallComplete(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}
};