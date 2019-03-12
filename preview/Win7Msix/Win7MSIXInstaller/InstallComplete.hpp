#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class InstallComplete : IPackageHandler
{
public:
	HRESULT ExecuteForAddRequest();

	static const PCWSTR HandlerName;
	static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
	~InstallComplete() {}
private:
	MsixRequest * m_msixRequest = nullptr;

	InstallComplete() {}
	InstallComplete(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}
};