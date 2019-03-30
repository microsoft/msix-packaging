#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "Protocol.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR Protocol::HandlerName = L"Protocol";

HRESULT Protocol::ExecuteForAddRequest()
{
    if (m_msixRequest->GetIsInstallCancelled())
    {
        return ERROR_INSTALL_USEREXIT;
    }

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

        if (wcscmp(extensionCategory.Get(), L"windows.protocol") == 0)
        {
            BOOL hc_protocol;
            ComPtr<IMsixElementEnumerator> protocolEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(L"*[local-name()='Protocol']", &protocolEnum));
            RETURN_IF_FAILED(protocolEnum->GetHasCurrent(&hc_protocol));

            if (hc_protocol)
            {
                ComPtr<IMsixElement> protocolElement;
                RETURN_IF_FAILED(protocolEnum->GetCurrent(&protocolElement));

                Text<wchar_t> name;
                RETURN_IF_FAILED(protocolElement->GetAttributeValue(L"Name", &name));

            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    if (m_msixRequest->GetIsInstallCancelled())
    {
        return ERROR_INSTALL_USEREXIT;
    }
    
    return S_OK;
}

HRESULT Protocol::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<Protocol> localInstance(new Protocol(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
