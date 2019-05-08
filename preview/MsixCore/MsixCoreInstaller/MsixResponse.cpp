#include "GeneralUtil.hpp"
#include "MsixResponse.hpp"
using namespace MsixCoreLib;

void MsixResponse::Update(InstallationStep status, float progress)
{
    if (m_percentage == progress && m_status == status)
    {
        return;
    }

    m_percentage = progress;
    m_status = status;
    if (m_callback)
    {
        m_callback(*this);
    }
}

void MsixResponse::SetCallback(std::function<void(const IMsixResponse& sender)> callback)
{
    m_callback = callback;
}

void MsixResponse::SetErrorStatus(HRESULT errorCode, std::wstring errorText)
{
    m_percentage = 0;
    m_status = InstallationStep::InstallationStepError;
    m_hresultTextCode = errorCode;
    m_textStatus = errorText;

    if (m_callback)
    {
        m_callback(*this);
    }
}
