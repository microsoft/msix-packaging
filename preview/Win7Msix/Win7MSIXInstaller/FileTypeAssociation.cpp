#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "FileTypeAssociation.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR FileTypeAssociation::HandlerName = L"FileTypeAssociation";

HRESULT FileTypeAssociation::AddFta(PCWSTR name, PCWSTR parameters)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Adding FTA",
        TraceLoggingValue(name, "Name"),
        TraceLoggingValue(parameters, "Parameters"));

    return S_OK;
}

HRESULT FileTypeAssociation::ExecuteForAddRequest()
{
    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(m_msixRequest->GetPackageInfo()->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    ComPtr<IMsixElementEnumerator> extensionEnum;
    RETURN_IF_FAILED(element->GetElements(L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='Extensions']/*[local-name()='Extension']", &extensionEnum));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(extensionEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> extensionElement;
        RETURN_IF_FAILED(extensionEnum->GetCurrent(&extensionElement));
        Text<wchar_t> extensionCategory;
        RETURN_IF_FAILED(extensionElement->GetAttributeValue(L"Category", &extensionCategory));

        if (wcscmp(extensionCategory.Get(), L"windows.fileTypeAssociation") == 0)
        {
            BOOL hc_fta;
            ComPtr<IMsixElementEnumerator> ftaEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(L"*[local-name()='FileTypeAssociation']", &ftaEnum));
            RETURN_IF_FAILED(ftaEnum->GetHasCurrent(&hc_fta));

            if (hc_fta)
            {
                ComPtr<IMsixElement> ftaElement;
                RETURN_IF_FAILED(ftaEnum->GetCurrent(&ftaElement));

                Text<wchar_t> ftaName;
                RETURN_IF_FAILED(ftaElement->GetAttributeValue(L"Name", &ftaName));

                Text<wchar_t> ftaParameters;
                RETURN_IF_FAILED(ftaElement->GetAttributeValue(L"Parameters", &ftaParameters));

                RETURN_IF_FAILED(AddFta(ftaName.Get(), ftaParameters.Get()));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT FileTypeAssociation::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<FileTypeAssociation> localInstance(new FileTypeAssociation(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
