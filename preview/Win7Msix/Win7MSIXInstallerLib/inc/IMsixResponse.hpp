#pragma once
#include <functional>
#include <windows.h>

namespace Win7MsixInstallerLib {

    enum InstallationStep {
        InstallationStepUnknown,
        InstallationStepStarted,
        InstallationStepGetPackageInformation,
        InstallationStepExtraction,
        InstallationStepCompleted,
        InstallationStepError,
        //can be extend for future scenarios.
    };

    class IMsixResponse
    {
    public:
        virtual void SetCallback(std::function<void(IMsixResponse *)> callback) = 0;
        virtual inline float GetPercentage() = 0;
        virtual inline InstallationStep GetStatus() = 0;
        virtual inline HRESULT GetHResultTextCode() = 0;
        virtual inline std::wstring GetTextStatus() = 0;
        virtual inline void CancelRequest() = 0;
    };
}
