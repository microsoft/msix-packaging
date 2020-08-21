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
    class CreateCIMDll
    {
        public:
            CreateCIMDll::CreateCIMDll()
            {

            }

            CreateCIMDll::~CreateCIMDll()
            {
                if (module != nullptr)
                {
                    FreeLibrary(module);
                }
            }

            HRESULT load()
            {
                module = LoadLibrary(L"createcim.dll");
                if (module == nullptr)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to load createcim.dll. Please confirm the dll is next to this exe." << std::endl;
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

    HRESULT CreateAndAddToCIM(
        _In_ std::wstring cimPath,
        _In_ std::wstring sourcePath,
        _In_ std::wstring rootDirectory)
    {
        CreateCIMDll createCIM;
        RETURN_IF_FAILED(createCIM.load());

        typedef HRESULT(STDMETHODCALLTYPE *CREATEANDADDTOCIMFILE)(
            std::wstring cimFilePath,
            std::wstring sourceRootPath,
            std::wstring imageRootPath);

        CREATEANDADDTOCIMFILE CreateAndAddToCimFileFunc =
            reinterpret_cast<CREATEANDADDTOCIMFILE>
            (GetProcAddress(createCIM.get(), "CreateAndAddToCIMFile"));
        
        RETURN_IF_FAILED(CreateAndAddToCimFileFunc(cimPath, sourcePath, rootDirectory));

        return S_OK;
    }

    HRESULT MountCIM(
        std::wstring cimFilePath,
        GUID volumeId)
    {
        CreateCIMDll createCIM;
        RETURN_IF_FAILED(createCIM.load());

        typedef HRESULT(STDMETHODCALLTYPE *MOUNTCIM)(
            std::wstring cimFilePath,
            GUID volumeId);

        MOUNTCIM MountCIMFunc =
            reinterpret_cast<MOUNTCIM>
            (GetProcAddress(createCIM.get(), "MountCIM"));

        RETURN_IF_FAILED(MountCIMFunc(cimFilePath, volumeId));

        return S_OK;
    }

    HRESULT UnmountCIM(
        GUID volumeId)
    {
        CreateCIMDll createCIM;
        RETURN_IF_FAILED(createCIM.load());

        typedef HRESULT(STDMETHODCALLTYPE *UNMOUNTCIM)(
            GUID volumeId);

        UNMOUNTCIM UnmountCIMFunc =
            reinterpret_cast<UNMOUNTCIM>
            (GetProcAddress(createCIM.get(), "UnmountCIM"));

        RETURN_IF_FAILED(UnmountCIMFunc(volumeId));

        return S_OK;
    }

}