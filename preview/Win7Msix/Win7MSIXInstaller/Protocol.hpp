#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include <vector>

/// Data structs to be filled in from the information in the manifest
struct ProtocolData
{
    std::wstring name;
    std::wstring displayName;
    std::wstring logo;
};

class Protocol : IPackageHandler
{
public:
    
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~Protocol() {}
private:
    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;
    std::vector<ProtocolData> m_protocols;

    Protocol() {}
    Protocol(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    HRESULT ParseProtocolElement(IMsixElement * protocolElement);

    HRESULT ParseManifest();

    HRESULT ProcessProtocolForAdd(ProtocolData & protocol);
};
    