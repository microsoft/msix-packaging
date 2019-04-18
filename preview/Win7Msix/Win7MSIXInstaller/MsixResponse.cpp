#include "GeneralUtil.hpp"
#include "MsixResponse.hpp"
using namespace Win7MsixInstallerLib;

HRESULT MsixResponse::Make(MsixResponse ** outInstance)
{
    std::unique_ptr<MsixResponse> instance(new MsixResponse());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    instance->m_hresultTextCode = NULL;
    instance->m_isInstallCancelled = false;
    *outInstance = instance.release();

    return S_OK;
}

void MsixResponse::Update(InstallationStep status, float progress)
{
    m_percentage = progress;
    m_status = status;
    if (m_callback)
    {
        m_callback((IMsixResponse *)this);
    }
}

void MsixResponse::SetCallback(std::function<void(IMsixResponse * sender)> callback)
{
    m_callback = callback;
}