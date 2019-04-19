#include "GeneralUtil.hpp"
#include "MsixResponse.hpp"
using namespace Win7MsixInstallerLib;

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