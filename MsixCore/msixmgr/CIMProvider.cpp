#include "MSIXWindows.hpp"
#include "CIMProvider.hpp"
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
    HRESULT CreateAndAddToCIM(
        _In_ std::wstring cimPath,
        _In_ std::wstring sourcePath,
        _In_ std::wstring rootDirectory)
    {
        auto autoFreeLibrary = [](HMODULE* module)
        {
            if (module != nullptr)
            {
                FreeLibrary(*module);
            }
        };

        std::unique_ptr<HMODULE, decltype(autoFreeLibrary)>
            createCIMDll(nullptr, autoFreeLibrary);

        HMODULE createCIMLocal = LoadLibrary(L"createcim.dll");
        createCIMDll.reset(&createCIMLocal);

        if (*createCIMDll == nullptr)
        {
            std::wcout << "Failed to load createcim.dll. Please confirm the dll is next to this exe." << std::endl;
            return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
        }

        typedef HRESULT(STDMETHODCALLTYPE *CREATEANDADDTOCIMFILE)(
            std::wstring cimFilePath,
            std::wstring sourceRootPath,
            std::wstring imageRootPath);

        CREATEANDADDTOCIMFILE CreateAndAddToCimFileFunc =
            reinterpret_cast<CREATEANDADDTOCIMFILE>
            (GetProcAddress(*createCIMDll, "CreateAndAddToCIMFile"));
        
        RETURN_IF_FAILED(CreateAndAddToCimFileFunc(cimPath, sourcePath, rootDirectory));

        return S_OK;
    }

    HRESULT MountCIM(
        std::wstring cimFilePath,
        GUID volumeId)
    {
        auto autoFreeLibrary = [](HMODULE* module)
        {
            if (module != nullptr)
            {
                FreeLibrary(*module);
            }
        };

        std::unique_ptr<HMODULE, decltype(autoFreeLibrary)>
            createCIMDll(nullptr, autoFreeLibrary);

        HMODULE createCIMLocal = LoadLibrary(L"createcim.dll");
        createCIMDll.reset(&createCIMLocal);

        if (*createCIMDll == nullptr)
        {
            std::wcout << "Failed to load createcim.dll. Please confirm the dll is next to this exe." << std::endl;
            return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
        }

        typedef HRESULT(STDMETHODCALLTYPE *MOUNTCIM)(
            std::wstring cimFilePath,
            GUID volumeId);

        MOUNTCIM MountCIMFunc =
            reinterpret_cast<MOUNTCIM>
            (GetProcAddress(*createCIMDll, "MountCIM"));

        RETURN_IF_FAILED(MountCIMFunc(cimFilePath, volumeId));

        return S_OK;
    }

    HRESULT UnmountCIM(
        GUID volumeId)
    {
        auto autoFreeLibrary = [](HMODULE* module)
        {
            if (module != nullptr)
            {
                FreeLibrary(*module);
            }
        };

        std::unique_ptr<HMODULE, decltype(autoFreeLibrary)>
            createCIMDll(nullptr, autoFreeLibrary);

        HMODULE createCIMLocal = LoadLibrary(L"createcim.dll");
        createCIMDll.reset(&createCIMLocal);

        if (*createCIMDll == nullptr)
        {
            std::wcout << "Failed to load createcim.dll. Please confirm the dll is next to this exe." << std::endl;
            return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
        }

        typedef HRESULT(STDMETHODCALLTYPE *UNMOUNTCIM)(
            GUID volumeId);

        UNMOUNTCIM UnmountCIMFunc =
            reinterpret_cast<UNMOUNTCIM>
            (GetProcAddress(*createCIMDll, "UnmountCIM"));

        RETURN_IF_FAILED(UnmountCIMFunc(volumeId));

        return S_OK;
    }

}