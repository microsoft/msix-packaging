//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "Exceptions.hpp"
#include <string>
#include <array>
#include <set>
#include <memory>
#include <regex>

namespace MSIX {

    // Used for validation of AppxPackageId name and resourceId
    static const std::size_t ProhibitedStringsSize = 24;
    static const std::array<const char*, ProhibitedStringsSize> ProhibitedStrings = {
        u8".",
        u8"..",
        u8"con",
        u8"prn",
        u8"aux",
        u8"nul",
        u8"com1",
        u8"com2",
        u8"com3",
        u8"com4",
        u8"com5",
        u8"com6",
        u8"com7",
        u8"com8",
        u8"com9",
        u8"lpt1",
        u8"lpt2",
        u8"lpt3",
        u8"lpt4",
        u8"lpt5",
        u8"lpt6",
        u8"lpt7",
        u8"lpt8",
        u8"lpt9",
    };

    static const std::size_t ProhibitedStringsBeginWithSize = 23;
    static const std::array<const char*, ProhibitedStringsBeginWithSize> ProhibitedStringsBeginWith = {
        u8"con.",
        u8"prn.",
        u8"aux.",
        u8"nul.",
        u8"com1.",
        u8"com2.",
        u8"com3.",
        u8"com4.",
        u8"com5.",
        u8"com6.",
        u8"com7.",
        u8"com8.",
        u8"com9.",
        u8"lpt1.",
        u8"lpt2.",
        u8"lpt3.",
        u8"lpt4.",
        u8"lpt5.",
        u8"lpt6.",
        u8"lpt7.",
        u8"lpt8.",
        u8"lpt9.",
        u8"xn--",
    };

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
            std::regex nameRegex("[a-zA-Z0-9\\.\\-]+"); // valid characters for name
            ValidatePackageString(Name);
            // If ResourceId == "~" this is the identity of a bundle.
            if (!ResourceId.empty() && ResourceId != "~")
            {
                ValidatePackageString(ResourceId);
            }
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
    private:
        void ValidatePackageString(std::string& packageString)
        {
            std::regex e("[a-zA-Z0-9\\.\\-]+"); // valid characters
            ThrowErrorIf(Error::AppxManifestSemanticError, !std::regex_match(packageString, e), "Invalid Package String");
            std::string packageStringLower;
            packageStringLower.resize(packageString.size());
            std::transform(packageString.begin(), packageString.end(), packageStringLower.begin(), ::tolower);
            // Package string can't be the same as any of the strings in ProhibitedStrings
            for(const auto& prohibited : ProhibitedStrings)
            {
                if(strlen(prohibited) == packageStringLower.size())
                {
                    ThrowErrorIf(Error::AppxManifestSemanticError, strcmp(prohibited, packageStringLower.c_str()) == 0,
                        "Invalid Package String");
                }
            }
            // Package string can't be begin with the strings in ProhibitedStringsBeginWith
            for(const auto& prohibited : ProhibitedStringsBeginWith)
            {
                if(strlen(prohibited) <= packageStringLower.size())
                {
                    ThrowErrorIf(Error::AppxManifestSemanticError, strncmp(prohibited, packageStringLower.c_str(), strlen(prohibited)) == 0,
                        "Invalid Package String");
                }
            }
            // Package string can't contain ".xn--"
            ThrowErrorIf(Error::AppxManifestSemanticError, strstr(".xn--", packageStringLower.c_str()) != nullptr,
                        "Invalid Package String");
            // Package string can't end with "."
            ThrowErrorIf(Error::AppxManifestSemanticError, packageStringLower[packageStringLower.size() - 1]  == '.',
                        "Invalid Package String");
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