#pragma once

#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
    enum AutoPlayType
    {
        InvalidAutoPlayType = 0,
        UWPContent,
        UWPDevice
    };

    /// the autoplay structure
    struct AutoPlayObject
    {
        std::wstring id;
        std::wstring action;
        std::wstring handleEvent;
        std::wstring provider;
        std::wstring defaultIcon;
        std::wstring appUserModelId;
        std::wstring generatedProgId;
        std::wstring generatedhandlerName;
        AutoPlayType autoPlayType;
    };

    class AutoPlay : IPackageHandler
    {
    public:
        HRESULT ExecuteForAddRequest();

        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    private:
        MsixRequest * m_msixRequest = nullptr;
        std::vector<AutoPlayObject> m_autoPlay;

        AutoPlay() {}
        AutoPlay(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        HRESULT ParseManifest();

        HRESULT ProcessAutoPlayForAdd(AutoPlayObject& autoPlayObject);

        HRESULT GenerateProgId(_In_ std::wstring categoryName, _In_opt_ std::wstring subCategory, _Out_ std::wstring generatedProgId);

        HRESULT GenerateHandlerName(_In_ LPWSTR type, _In_ const std::wstring handlerNameSeed, _Out_ std::wstring generatedHandlerName);

        static ULONG GetMinValue(
            _In_ ULONG d1,
            _In_ ULONG d2)
        {
            return (d1 < d2 ? d1 : d2);
        }

    };
}