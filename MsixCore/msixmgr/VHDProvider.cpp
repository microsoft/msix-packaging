#include "MSIXWindows.hpp"
#include "VHDProvider.hpp"
#include "msixmgrLogger.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"
#include <string>
#include <iostream>
#include "InstallUI.hpp"

using namespace MsixCoreLib;
using namespace std;

namespace MsixCoreLib
{

    class WVDUtilitiesDll
    {
    public:
        WVDUtilitiesDll::WVDUtilitiesDll()
        {

        }

        WVDUtilitiesDll::~WVDUtilitiesDll()
        {
            if (module != nullptr)
            {
                FreeLibrary(module);
            }
        }

        HRESULT load()
        {
            module = LoadLibrary(L"wvdutilities.dll");
            if (module == nullptr)
            {
                std::wcout << std::endl;
                std::wcout << "Failed to load wvdutilities.dll. Please confirm the dll is next to this exe." << std::endl;
                std::wcout << std::endl;

                return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
            }
            return S_OK;
        }

        HMODULE get()
        {
            return module;
        }

    private:
        HMODULE module;
    };

    HRESULT CreateAndMountVHD(
        _In_ const std::wstring& vhdFilePath,
        _In_ ULONGLONG sizeMBs,
        _In_ bool isVHD,
        _Inout_ std::wstring& driveLetter)
    {
        WVDUtilitiesDll wvdUtilities;
        RETURN_IF_FAILED(wvdUtilities.load());

        typedef HRESULT(STDMETHODCALLTYPE *CREATEANDMOUNTVHD)(
            const std::wstring& vhdFilePath,
            ULONGLONG sizeMBs,
            bool isVHD,
            std::wstring& driveLetter);

        CREATEANDMOUNTVHD CreateAndMountVHD =
            reinterpret_cast<CREATEANDMOUNTVHD>
            (GetProcAddress(wvdUtilities.get(), "CreateAndMountVHD"));

        RETURN_IF_FAILED(CreateAndMountVHD(vhdFilePath, sizeMBs, isVHD, driveLetter));

        return S_OK;
    }

    HRESULT UnmountVHD(
        _In_ const std::wstring& vhdFilePath)
    {
        WVDUtilitiesDll wvdUtilities;
        RETURN_IF_FAILED(wvdUtilities.load());

        typedef HRESULT(STDMETHODCALLTYPE *UNMOUNTVHD)(const std::wstring& vhdFilePath);

        UNMOUNTVHD UnmountVHD =
            reinterpret_cast<UNMOUNTVHD>
            (GetProcAddress(wvdUtilities.get(), "UnmountVHD"));

        RETURN_IF_FAILED(UnmountVHD(vhdFilePath));

        return S_OK;
    }

    HRESULT MountVHD(
        _In_ const std::wstring& vhdFilePath,
        _In_ bool readOnly,
        _Inout_ std::wstring& driveLetter)
    {
        WVDUtilitiesDll wvdUtilities;
        RETURN_IF_FAILED(wvdUtilities.load());

        typedef HRESULT(STDMETHODCALLTYPE *MOUNTVHD)(const std::wstring& vhdFilePath, bool readOnly, std::wstring& driveLetter);

        MOUNTVHD MountVHD =
            reinterpret_cast<MOUNTVHD>
            (GetProcAddress(wvdUtilities.get(), "MountVHD"));

        RETURN_IF_FAILED(MountVHD(vhdFilePath, readOnly, driveLetter));

        return S_OK;
    }
}