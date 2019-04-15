#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class ValidateRequest : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ValidateRequest() {}
private:
    MsixRequest * m_msixRequest = nullptr;

    std::wstring m_targetDeviceFamilyName;
    std::wstring m_minVersion;

    ValidateRequest() {}
    ValidateRequest(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    HRESULT ParseDependencyInfoFromPackage();
    HRESULT MatchTDFAndVersionToOS();
};
