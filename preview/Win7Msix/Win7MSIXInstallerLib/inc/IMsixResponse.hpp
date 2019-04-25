#pragma once
#include <functional>
#include <windows.h>

namespace MsixCoreLib {

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
        virtual void SetCallback(std::function<void(const IMsixResponse &)> callback) = 0;
        virtual inline float GetPercentage() const = 0;
        virtual inline InstallationStep GetStatus() const  = 0;
        virtual inline HRESULT GetHResultTextCode() const = 0;
        virtual inline std::wstring GetTextStatus() const = 0;
        virtual inline void CancelRequest() = 0;
    };
}
