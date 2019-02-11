#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"

class FileTypeAssociation : IPackageHandler
{
public:
    /// Adds the file type associations to the registry so this application can handle specific file types.
    HRESULT ExecuteForAddRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~FileTypeAssociation() {}
private:
    MsixRequest* m_msixRequest = nullptr;

    FileTypeAssociation() {}
    FileTypeAssociation(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    HRESULT AddFta(PCWSTR name, PCWSTR parameters);
};
