#pragma once
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include <vector>

struct Interface
{
    std::wstring id;
    std::wstring proxyStubClsid;
    std::wstring typeLibId;
    std::wstring typeLibVersion;
};

class ComInterface : IPackageHandler
{
public:
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ComInterface() {}
private:
    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;

    std::vector<Interface> m_interfaces;

    /// Parses the manifest and fills in the m_ComInfos vector of ComInfo data
    HRESULT ParseManifest();

    /// Parses the manifest element to populate one Interface struct entry of the m_ComInfos vector
    /// 
    /// @param interfaceElement - the manifest element representing an interface
    HRESULT ParseComInterfaceElement(IMsixElement* interfaceElement);

    ComInterface() {}
    ComInterface(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

};
