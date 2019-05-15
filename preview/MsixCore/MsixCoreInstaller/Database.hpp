#pragma once
#include "inc/IPackage.hpp"
#include <vector>

namespace MsixCoreLib
{
    /// This is the database interface for tracking which packages each user has installed.
    /// We are using a simple registry tree to list the users and packages they have installed.
    class Database
    {
    public:
        /// Adds the user and package for the user to the registry
        /// @param user - the user
        /// @param packageFullName - the package
        static HRESULT AddPackageForUser(PCWSTR user, PCWSTR packageFullName);

        /// Adds the current user and the package for the current user to the registry
        /// @param packageFullName - the package
        static HRESULT AddPackageForCurrentUser(PCWSTR packageFullName);

        /// Removes the package for the user from the registry
        /// @param user - the user
        /// @param packageFullName - the package
        static HRESULT RemovePackageForUser(PCWSTR user, PCWSTR packageFullName);

        /// Removes the package for the current user from the registry
        /// @param packageFullName - the package
        static HRESULT RemovePackageForCurrentUser(PCWSTR packageFullName);

        /// Determines if the package is installed for any other user besides the current user.
        /// @param packageFullName - the package
        /// @param[out] isInstalled - if the package is installed for another user
        static HRESULT IsInstalledForAnyOtherUser(PCWSTR packageFullName, bool& isInstalled);

        /// Finds all packages installed for the current user
        /// @param[out] installedPackages - the list of packages installed for the current user
        static HRESULT FindPackagesForCurrentUser(std::vector<std::wstring> & installedPackages);
    };
}