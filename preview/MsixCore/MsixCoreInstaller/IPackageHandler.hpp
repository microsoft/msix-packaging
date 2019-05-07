#pragma once
#include "Package.hpp"
#include "MsixRequest.hpp"

namespace MsixCoreLib
{
/// Interface for a logical chunk of work done on an Msix request
class IPackageHandler
{
public:

    // Add request follows the hybrid per user, where files are written into a per-machine shared location,
    // but OS integration points are written to per-user visible locations so one user's adds is not visible to other users.
    virtual HRESULT ExecuteForAddRequest() = 0; 

    // Removes packages added by ExecuteForAddRequest. 
    virtual HRESULT ExecuteForRemoveRequest() { return S_OK; }

    // Add-for-all-users adds the package as per-machine. Everything is written to per-machine shared locations.
    virtual HRESULT ExecuteForAddForAllUsersRequest() = 0;

    // Remove-for-all-users removes packages that were added using ExecuteForAddForAllUsersRequest.
    // This does not work for packages that were per-user added.
    virtual HRESULT ExecuteForRemoveForAllUsersRequest() { return S_OK; }

    virtual bool IsMandatoryForRemoveRequest() { return false; }
    virtual bool IsMandatoryForAddRequest() { return true; }

    virtual ~IPackageHandler() {};
};

/// Function responsible for creating an instance of an IPackageHandler object 	/// Function responsible for creating an instance of an IPackageHandler object 
/// @param msixRequest - the msix request object to act upon	/// @return S_OK    of the package.
/// @return S_OK if CreateHandler is to not fail the deployment of the package.
typedef HRESULT(*CreateHandler)(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
}