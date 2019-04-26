#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include <vector>
#include "MsixRequest.hpp"

namespace MsixCoreLib
{

/// Data structs to be filled in from the information in the manifest
struct ProtocolData
{
    std::wstring name;          // name of the URI scheme, such as "mailto"
    std::wstring parameters;    // parameters to be passed into the executable when invoked
    std::wstring displayName;   // friendly name to be displayed to users
    std::wstring logo;          // filepath to logo file 
};

class Protocol : IPackageHandler
{
public:
    
    HRESULT ExecuteForAddRequest();
    HRESULT ExecuteForRemoveRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~Protocol() {}
private:
    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;
    std::vector<ProtocolData> m_protocols;

    Protocol() {}
    Protocol(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Parse one protocol element, containing one ProtocolData to be added to the m_protocols vector.
    ///
    /// @param protocolElement - the IMsixElement representing the uap:Protocol element from the manifest
    HRESULT ParseProtocolElement(IMsixElement * protocolElement);

    /// Parses the manifest and fills out the m_protocols vector of ProtocolData containing data from the manifest
    HRESULT ParseManifest();

    /// Adds the protocol data to the system registry
    ///
    /// @param protocol - the protocol data to be added
    HRESULT ProcessProtocolForAdd(ProtocolData & protocol);

    /// Removes the protocol data from the system registry
    ///
    /// @param protocol - the protocol data to be removed
    HRESULT ProcessProtocolForRemove(ProtocolData& protocol);

    /// Determines if the protocol is currently associated with the package
    ///
    /// @param name - the name of the protocol
    bool IsCurrentlyAssociatedWithPackage(PCWSTR name);
};
}    