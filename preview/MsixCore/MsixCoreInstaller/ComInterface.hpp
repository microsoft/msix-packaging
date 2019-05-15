#pragma once
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include "MsixRequest.hpp"
#include <vector>

namespace MsixCoreLib
{
class ComInterface : IPackageHandler
{
public:
    /// Adds the com interface registrations to the per-user registry
    HRESULT ExecuteForAddRequest();

    /// Removes the com interface registrations from the per-user registry.
    HRESULT ExecuteForRemoveRequest();

    /// Adds the com interface registrations to the per-machine registry
    HRESULT ExecuteForAddForAllUsersRequest();

    /// Removes the com interface registrations from the per-machine registry.
    HRESULT ExecuteForRemoveForAllUsersRequest();

    static const PCWSTR HandlerName;
    
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ComInterface() {}
private:

    struct Interface
    {
        std::wstring id;
        std::wstring proxyStubClsid;
        std::wstring typeLibId;
        std::wstring typeLibVersion;
    };

    struct Version
    {
        std::wstring displayName;
        std::wstring versionNumber;
        std::wstring localeId;
        std::wstring libraryFlag;
        std::wstring helpDirectory;
        std::wstring win32Path;
        std::wstring win64Path;
    };

    struct TypeLib
    {
        std::wstring id;
        std::vector<Version> version;
    };

    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;

    std::vector<Interface> m_interfaces;
    std::vector<TypeLib> m_typeLibs;

    /// Parses the manifest and fills in the m_interfaces and m_typeLibs data
    HRESULT ParseManifest();

    /// Parses the manifest element to populate one Interface struct entry of the m_interfaces vector
    /// 
    /// @param interfaceElement - the manifest element representing an interface
    HRESULT ParseComInterfaceElement(IMsixElement* interfaceElement);

    /// Parses the manifest element to populate one TypeLib struct entry of the m_TypeLibs vector
    /// 
    /// @param typeLibElement - the manifest element representing an typeLib
    HRESULT ParseTypeLibElement(IMsixElement* typeLibElement);

    /// Parses the manifest element to populate one Version struct for one of the type libs
    ///
    /// @param typeLib - a TypeLib for which to populate a version
    /// @param versionElement - the version element from the manifest
    HRESULT ParseVersionElement(TypeLib & typeLib, IMsixElement * versionElement);

    /// Adds the registry entries corresponding to an Interface
    ///
    /// @param comInterface - Interface struct representing data from the manifest
    HRESULT ProcessInterfaceForAddRequest(Interface & comInterface);

    /// Adds the registry entries corresponding to an TypeLib
    ///
    /// @param typeLib - TypeLib struct representing data from the manifest
    HRESULT ProcessTypeLibForAddRequest(TypeLib & typeLib);

    /// Removes the registry entries corresponding to an Interface
    ///
    /// @param comInterface - Interface struct representing data from the manifest
    HRESULT ProcessInterfaceForRemoveRequest(Interface & comInterface);

    /// Removes the registry entries corresponding to an TypeLib
    ///
    /// @param typeLib - TypeLib struct representing data from the manifest
    HRESULT ProcessTypeLibForRemoveRequest(TypeLib & typeLib);

    /// Adds all the registry entries associated with the interfaces and typelibs
    HRESULT AddInterfacesAndTypeLibs();

    /// Removes all the registry entries associated with the interfaces and typelibs
    HRESULT RemoveInterfacesAndTypeLibs();

    ComInterface() {}
    ComInterface(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

};
}