//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <regex>
#include <string>

#include "AppxManifestValidation.hpp"
#include "AppxPackageInfo.hpp"
#include "Exceptions.hpp"
#include "StringHelper.hpp"
#include "IXml.hpp"

namespace MSIX {

    namespace
    {
        struct TargetAttribute
        {
            XmlQueryName ElementQuery;
            XmlAttributeName AttributeName;
        };

        struct TargetAttributeComparison
        {
            XmlQueryName ElementQuery;
            XmlAttributeName FirstAttributeName;
            XmlAttributeName SecondAttributeName;
        };

        bool GetPropertyAsBool(IXmlDom* manifest, XmlQueryName query)
        {
            bool result = false;
            XmlVisitor visitor{ &result,
                [](void* c, const MSIX::ComPtr<IXmlElement>& element)
            {
                *reinterpret_cast<bool*>(c) = (element->GetText() == "true");
                return false;
            }};
            manifest->ForEachElementIn(query, visitor);
            return result;
        }

        bool AreAnyPresent(IXmlDom* manifest, XmlQueryName query)
        {
            bool result = false;
            XmlVisitor visitor{ &result,
                [](void* c, const MSIX::ComPtr<IXmlElement>& element)
            {
                *reinterpret_cast<bool*>(c) = true;
                return false;
            } };
            manifest->ForEachElementIn(query, visitor);
            return result;
        }

        bool AreAnyPresent(IXmlDom* manifest, XmlQueryName query, XmlAttributeName attribute)
        {
            struct context
            {
                bool result;
                XmlAttributeName attribute;
            };
            context thisContext{ false, attribute };

            XmlVisitor visitor{ &thisContext,
                [](void* c, const MSIX::ComPtr<IXmlElement>& element)
            {
                context& thisContext = *reinterpret_cast<context*>(c);

                std::string attributeValue = element->GetAttributeValue(thisContext.attribute);

                if (!attributeValue.empty())
                {
                    thisContext.result = true;
                    return false;
                }

                return true;
            } };
            manifest->ForEachElementIn(query, visitor);
            return thisContext.result;
        }

#pragma region ValidateIdentifiers

        const char* ProhibitedFileNames[] = {
            ".",
            "..",
            "con",
            "prn",
            "aux",
            "nul",
            "com1",
            "com2",
            "com3",
            "com4",
            "com5",
            "com6",
            "com7",
            "com8",
            "com9",
            "lpt1",
            "lpt2",
            "lpt3",
            "lpt4",
            "lpt5",
            "lpt6",
            "lpt7",
            "lpt8",
            "lpt9",
        };

        // Caller must pass in a lowercase string!
        bool IsProhibitedFileName(const std::string& identifier)
        {
            for (const auto& name : ProhibitedFileNames)
            {
                if (identifier == name)
                {
                    return true;
                }
            }
            return false;
        }

        const std::string ProhibitedPrefixes[] = {
            "con.",
            "prn.",
            "aux.",
            "nul.",
            "com1.",
            "com2.",
            "com3.",
            "com4.",
            "com5.",
            "com6.",
            "com7.",
            "com8.",
            "com9.",
            "lpt1.",
            "lpt2.",
            "lpt3.",
            "lpt4.",
            "lpt5.",
            "lpt6.",
            "lpt7.",
            "lpt8.",
            "lpt9.",
            "xn--",
        };

        bool HasProhibitedPrefix(const std::string& identifier)
        {
            for (const auto& prefix : ProhibitedPrefixes)
            {
                if (identifier.size() >= prefix.size() &&
                    identifier.substr(0, prefix.size()) == prefix)
                {
                    return true;
                }
            }
            return false;
        }

        const std::string ProhibitedSuffixes[] = {
            ".",
        };

        bool HasProhibitedSuffix(const std::string& identifier)
        {
            for (const auto& suffix : ProhibitedSuffixes)
            {
                if (identifier.size() >= suffix.size() &&
                    identifier.substr(identifier.size() - suffix.size()) == suffix)
                {
                    return true;
                }
            }
            return false;
        }

        void ValidateIdentifier(const TargetAttribute& target, const MSIX::ComPtr<IXmlElement>& element)
        {
            std::string attributeValue = element->GetAttributeValue(target.AttributeName);

            if (!attributeValue.empty())
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, !AppxManifestValidation::IsIdentifierValid(attributeValue),
                    (std::string("Invalid Identifier ") + GetAttributeNameStringUtf8(target.AttributeName) + ": " + attributeValue).c_str());
            }
        }

        const TargetAttribute Identifiers[] = {
            { XmlQueryName::Any_Identity, XmlAttributeName::Name },
            { XmlQueryName::Any_Identity, XmlAttributeName::ResourceId },
            { XmlQueryName::Package_Dependencies_PackageDependency, XmlAttributeName::Name },
            { XmlQueryName::Package_Dependencies_MainPackageDependency, XmlAttributeName::Name },
        };

        // Validates that all identifiers throughout the manifest are valid.
        void ValidateIdentifiers(IXmlDom* manifest)
        {
            auto CallValidateIdentifier = [](void* target, const MSIX::ComPtr<IXmlElement>& element)
            {
                ValidateIdentifier(*reinterpret_cast<TargetAttribute*>(target), element);
                return true;
            };

            for (const auto& identifier : Identifiers)
            {
                XmlVisitor visitor{ &identifier, CallValidateIdentifier };
                manifest->ForEachElementIn(identifier.ElementQuery, visitor);
            }
        }

#pragma endregion

#pragma region ValidateDependencies

        void ValidateDependency(const TargetAttributeComparison& target, const MSIX::ComPtr<IXmlElement>& element)
        {
            std::string minValue = element->GetAttributeValue(target.FirstAttributeName);
            std::string maxValue = element->GetAttributeValue(target.SecondAttributeName);

            if (maxValue.empty())
            {
                // If max is empty, can't have an inversion.
                return;
            }

            auto minVersion = DecodeVersionNumber(minValue);
            auto maxVersion = DecodeVersionNumber(maxValue);

            if (minVersion > maxVersion)
            {
                std::string name = element->GetAttributeValue(XmlAttributeName::Name);
                ThrowErrorAndLog(Error::AppxManifestSemanticError,
                    (std::string("Minimum version greater than maximum tested version for dependency ") + name).c_str());
            }
        }

        const TargetAttributeComparison Dependencies[] = {
            { XmlQueryName::Package_Dependencies_TargetDeviceFamily, XmlAttributeName::MinVersion, XmlAttributeName::Dependencies_Tdf_MaxVersionTested },
            { XmlQueryName::Package_Dependencies_PackageDependency, XmlAttributeName::MinVersion, XmlAttributeName::MaxMajorVersionTested },
        };

        // Validates that the dependencies don't have inverted version declarations (min > max).
        void ValidateDependencies(IXmlDom* manifest)
        {
            auto CallValidateDependency = [](void* target, const MSIX::ComPtr<IXmlElement>& element)
            {
                ValidateDependency(*reinterpret_cast<TargetAttributeComparison*>(target), element);
                return true;
            };

            for (const auto& dependency : Dependencies)
            {
                XmlVisitor visitor{ &dependency, CallValidateDependency };
                manifest->ForEachElementIn(dependency.ElementQuery, visitor);
            }
        }

#pragma endregion

        const XmlQueryName InvalidFrameworkQueries[] = {
            XmlQueryName::Package_Applications,
            XmlQueryName::Package_Capabilities,
        };

        // Validates that certain elements are not defined in a framework package.
        void ValidateFrameworkPackage(IXmlDom* manifest)
        {
            for (const auto& query : InvalidFrameworkQueries)
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, AreAnyPresent(manifest, query),
                    (std::string("A framework package cannot contain ") + GetQueryStringUtf8(query)).c_str());
            }
        }

        const XmlQueryName InvalidResourceElementQueries[] = {
            XmlQueryName::Package_Applications,
            XmlQueryName::Package_Capabilities,
            XmlQueryName::Package_Extensions,
            XmlQueryName::Package_Dependencies_PackageDependency,
            XmlQueryName::Package_Dependencies_MainPackageDependency,
        };

        const TargetAttribute InvalidResourceAttributeQueries[] = {
            { XmlQueryName::Package_Identity, XmlAttributeName::Identity_ProcessorArchitecture },
        };

        // Validates that certain elements are not defined in a resource package.
        void ValidateResourcePackage(IXmlDom* manifest)
        {
            for (const auto& query : InvalidResourceElementQueries)
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, AreAnyPresent(manifest, query),
                    (std::string("A resource package cannot contain ") + GetQueryStringUtf8(query)).c_str());
            }

            for (const auto& query : InvalidResourceAttributeQueries)
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, AreAnyPresent(manifest, query.ElementQuery, query.AttributeName),
                    (std::string("A resource package cannot contain ") + GetAttributeNameStringUtf8(query.AttributeName)).c_str());
            }
        }

        const XmlQueryName InvalidOptionalQueries[] = {
            XmlQueryName::Package_Capabilities,
            XmlQueryName::Package_Properties_SupportedUsers,
        };

        // Validates an optional package for it's unique constraints.
        void ValidateOptionalPackage(IXmlDom* manifest)
        {
            for (const auto& query : InvalidOptionalQueries)
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, AreAnyPresent(manifest, query),
                    (std::string("An optional package cannot contain ") + GetQueryStringUtf8(query)).c_str());
            }

            // TODO: Ensure lowest version is >= TH2... for Windows targets only

            // TODO: Ensure that there are not duplcate mainpackagedependencies
        }

        // Validates that there are not conflicting package types and then does specific validation for the type.
        void ValidatePackageType(IXmlDom* manifest)
        {
            bool isFramework = GetPropertyAsBool(manifest, XmlQueryName::Package_Properties_Framework);
            bool isResource = GetPropertyAsBool(manifest, XmlQueryName::Package_Properties_ResourcePackage);

            ThrowErrorIf(Error::AppxManifestSemanticError, isFramework && isResource,
                "Package cannot be both a framework and a resource");

            bool isOptional = AreAnyPresent(manifest, XmlQueryName::Package_Dependencies_MainPackageDependency);

            ThrowErrorIf(Error::AppxManifestSemanticError, isOptional && (isFramework || isResource),
                "Package cannot be optional if it is a framework or resource");

            if (isFramework)
            {
                ValidateFrameworkPackage(manifest);
            }
            else if (isResource)
            {
                ValidateResourcePackage(manifest);
            }
            else if (isOptional)
            {
                ValidateOptionalPackage(manifest);
            }
        }
    }

    bool AppxManifestValidation::IsIdentifierValid(const std::string& identifier)
    {
#if !VALIDATING
        // If the schema didn't check for us, do it now.
        std::regex e("[a-zA-Z0-9\\.\\-]+"); // valid characters
        if (!std::regex_match(identifier, e))
        {
            return false;
        }
#endif

        std::string lowIdent = Helper::tolower(identifier);
        return !IsProhibitedFileName(lowIdent) && !HasProhibitedPrefix(lowIdent) && !HasProhibitedSuffix(lowIdent);
    }

    void AppxManifestValidation::ValidateManifest(IXmlDom* manifest)
    {
        ValidateIdentifiers(manifest);
        ValidateDependencies(manifest);
        ValidatePackageType(manifest);

        //TODO
        //ValidateProperties(manifest);
        //ValidateResources(manifest);
        //ValidateApplicationContentURIs(manifest);
        //ValidateShareTargetExtensions(manifest);
        //ValidateUniqueWebAccountProviderURLs(manifest);
        //ValidatePackageExecutionTrust(manifest);
        //ValidateCapabilityByAttributeValue(manifest);
        //ValidateBinaryFileStringValues(manifest);
        //ValidateModifiedStringLength(manifest);
        //ValidateRequiredToOptionalAttributes(manifest);
        //ValidateUniqueCaseInsensitiveStrings(manifest);
        //ValidateExtensionCategories(manifest);
        //ValidatePackageProperties(manifest);
        //ValidateUniqueExtensions(manifest);
        //ValidatePackageExtensions(manifest);
        //ValidateApplicationObjects(manifest);
    }

}
