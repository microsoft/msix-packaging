#include "MSIXWindows.hpp"
#include "ApplyACLsProvider.hpp"
#include "msixmgrLogger.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <TraceLoggingProvider.h>
#include "InstallUI.hpp"

using namespace MsixCoreLib;
using namespace std;

namespace MsixCoreLib
{
    HRESULT ApplyACLs(
        _In_ std::vector<std::wstring> packageFolders)
    {
        auto autoFreeLibrary = [](HMODULE* module)
        {
            if (module != nullptr)
            {
                FreeLibrary(*module);
            }
        };

        std::unique_ptr<HMODULE, decltype(autoFreeLibrary)>
            applyACLsDll(nullptr, autoFreeLibrary);

        HMODULE applyACLsLocal = LoadLibrary(L"applyacls.dll");
        applyACLsDll.reset(&applyACLsLocal);

        if (*applyACLsDll == nullptr)
        {
            std::wcout << "Failed to load applyacls.dll. Please confirm the dll is next to this exe. ApplyACLs should only be used for setting up a Windows Virtual Desktop." << std::endl;
            return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
        }

        typedef HRESULT(STDMETHODCALLTYPE *APPLYACLSTOPACKAGEFOLDER)(PCWSTR folderPath);

        APPLYACLSTOPACKAGEFOLDER ApplyACLsToPackageFolder =
            reinterpret_cast<APPLYACLSTOPACKAGEFOLDER>
            (GetProcAddress(*applyACLsDll, "ApplyACLsToPackageFolder"));

        for (auto folder : packageFolders)
        {
            RETURN_IF_FAILED(ApplyACLsToPackageFolder(folder.c_str()));
        }

        return S_OK;
    }
}