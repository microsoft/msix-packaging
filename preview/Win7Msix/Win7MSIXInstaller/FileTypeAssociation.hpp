#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include "RegistryDevirtualizer.hpp"
#include "MsixRequest.hpp"

namespace Win7MsixInstallerLib
{
/// Data structs to be filled in from the information in the manifest
struct Verb
{
    std::wstring verb;
    std::wstring parameter;
};

struct Fta
{
    std::wstring name;
    std::wstring progID;
    std::vector<std::wstring> extensions;
    std::wstring logo;
    std::vector<Verb> verbs;
};

class FileTypeAssociation : IPackageHandler
{
public:
    /// Adds the file type associations to the registry so this application can handle specific file types.
    HRESULT ExecuteForAddRequest();

    /// Removes the file type associations from the registry.
    HRESULT ExecuteForRemoveRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~FileTypeAssociation() {}
private:
    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;
    AutoPtr<RegistryDevirtualizer> m_registryDevirtualizer; 
    std::vector<Fta> m_Ftas;

    FileTypeAssociation() {}
    FileTypeAssociation(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    /// Parses the manifest and fills in the m_Ftas vector of FileTypeAssociation (Fta) data
    HRESULT ParseManifest();

    /// Parses the manifest element to populate one Fta struct entry of the m_Ftas vector
    /// 
    /// @param ftaElement - the manifest element representing an Fta
    HRESULT ParseFtaElement(IMsixElement* ftaElement);

    /// Adds the file type association (fta) entries if necessary
    HRESULT ProcessFtaForAdd(Fta& fta);

    /// Removes the file type association (fta) entries if necessary
    HRESULT ProcessFtaForRemove(Fta& fta);

    /// Creates a ProgID from the name of the fta. Simply take the package name and prepend it to the fta
    std::wstring CreateProgID(PCWSTR name);
};
}