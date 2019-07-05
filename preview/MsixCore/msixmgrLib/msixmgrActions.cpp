#include "MsixCoreInstallerActions.hpp"
#include "PackageManager.hpp"

using namespace MsixCoreLib;

HRESULT MsixCoreLib_CreatePackageManager(MsixCoreLib::IPackageManager** packageManager)
{
    *packageManager = new PackageManager();
    return S_OK;
}
