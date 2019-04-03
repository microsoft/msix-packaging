#include "generalutil.hpp"
#include "MsixResponse.hpp"

HRESULT MsixResponse::Make(MsixResponse ** outInstance)
{
    std::unique_ptr<MsixResponse> instance(new MsixResponse());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    instance->m_hresultTextCode = NULL;
    instance->m_textStatus = NULL;
    instance->m_isInstallCancelled = false;
    *outInstance = instance.release();

    return S_OK;
}