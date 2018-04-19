//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "Exceptions.hpp"
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <regex>

namespace MSIX {
    // The 5-tuple that describes the identity of a package
    struct AppxPackageId
    {
        AppxPackageId(
            const std::string& name,
            const std::string& version,
            const std::string& resourceId,
            const std::string& architecture,
            const std::string& publisherId) :
            Name(name), Version(version), ResourceId(resourceId), Architecture(architecture), PublisherId(publisherId)
        {
            // Only name, publisherId and version are required.
            ThrowErrorIf(Error::AppxManifestSemanticError, (Name.empty() || Version.empty() || PublisherId.empty()), "Invalid Identity element");
            // TODO: validate the name and resource id as package strings
        }

        std::string Name;
        std::string Version;
        std::string ResourceId;
        std::string Architecture;
        std::string PublisherId;

        std::string GetPackageFullName()
        {
            return Name + "_" + Version + "_" + Architecture + "_" + ResourceId + "_" + PublisherId;
        }

        std::string GetPackageFamilyName()
        {
            return Name + "_" + PublisherId;
        }
    };

    struct AppxPackageInBundle
    {
        AppxPackageInBundle(
            const std::string& name,
            const std::string& bundleName,
            const std::string& version,
            const std::uint64_t size,
            const std::uint64_t offset,
            const std::string& resourceId,
            const std::string& architecture,
            const std::string& publisherId,
            bool packageType) :
            FileName(name), Size(size), Offset(offset), IsResourcePackage(packageType)
        {
            PackageId = std::make_unique<AppxPackageId>(bundleName, version, resourceId, architecture, publisherId);
            std::regex e (".+\\.((appx)|(msix))");
            ThrowErrorIf(Error::AppxManifestSemanticError, !std::regex_match(FileName, e), "Invalid FileName attribute in AppxBundleManifest.xml");
        }

        std::string FileName;
        std::unique_ptr<AppxPackageId> PackageId;
        std::uint64_t Size;
        std::uint64_t Offset;
        std::set<std::string> Languages;
        bool IsResourcePackage;
    };  
}