#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class Protocol : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~Protocol() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    Protocol() {}
    Protocol(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

};
    